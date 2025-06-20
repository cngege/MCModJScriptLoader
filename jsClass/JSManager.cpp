﻿#include "JSManager.h"
#include <fstream>
#include <iostream>
#include "../client/ModManager.h"
#include "../client/ConfigManager.h"
#include "spdlog/spdlog.h"
#include "eventSystem/JsModule_eventSystem.h"
#include <unordered_map>
#include <shared_mutex>
#include "imgui/appConsole.h"
#include "imgui/imgui.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "http/httplib.h"

#include <regex>
#include <chrono>
#include <map>
#include <mutex>
#include <queue>



struct TaskQueuesStruct
{
    /**
     * @brief 要执行的JS的回调函数
     */
    JSValue call;
    /**
     * @brief 0 setTimeout, 1: setInterval
     */
    std::byte type;
    /**
     * @brief 延迟的时间或者循环的间隔
     */
    int64_t time;
    /**
     * @brief 当前时间的64位整数表示。添加时候的时间以及记录的时间
     */
    int64_t curr_t;
};

static std::map<int64_t, TaskQueuesStruct> TaskQueues = {};
static std::recursive_mutex taskQueueMtx;
static std::priority_queue<int64_t, std::vector<int64_t>, std::greater<int64_t>> freeKeys; // 小顶堆
static int64_t nextKey = 1; // 下一个新键

static std::string JS_ErrorStackCheck(JSContext* ctx) {
    JSValue err = JS_GetException(ctx);
    std::string result_str = JSManager::getInstance()->getErrorStack(err);
    JS_FreeValue(ctx, err);
    return result_str;
}


JSManager* JSManager::getInstance() {
    static JSManager instance{};
    return &instance;
}

auto JSManager::setctx(JSContext* ctx) -> void {
    m_ctx = ctx;
}

auto JSManager::getctx() -> JSContext* const {
    return m_ctx;
}

auto JSManager::setrt(JSRuntime* rt) -> void {
    m_rt = rt;
}

auto JSManager::getrt() -> JSRuntime* const {
    return m_rt;
}

JSModuleDef* js_init_module_mem(JSContext* ctx, const char* module_name);
JSModuleDef* js_init_module_eventSystem(JSContext* ctx, const char* module_name);
JSModuleDef* js_init_module_imgui(JSContext* ctx, const char* module_name);
JSModuleDef* js_init_module_signcode(JSContext* ctx, const char* module_name);
JSModuleDef* js_init_module_http(JSContext* ctx, const char* module_name);


auto JSManager::loadNativeModule() -> void {
    js_init_module_std(m_ctx, "std");
    js_init_module_os(m_ctx, "os");
    js_init_module_bjson(m_ctx, "bjson");

    js_init_module_mem(m_ctx, "工具");
    js_init_module_eventSystem(m_ctx, "事件系统");
    js_init_module_imgui(m_ctx, "imgui");
    js_init_module_signcode(m_ctx, "SignCode");
    js_init_module_http(m_ctx, "http");
}

auto JSManager::freeNativeModule(JSRuntime* rt) -> void {
    js_std_free_handlers(rt);
}

auto JSManager::loadModuleFromFile(const std::string& path) -> JSModuleDef* {
    static std::unordered_map<std::size_t, JSModuleDef*> onlyLoad{};
    try {
        // 接收到的就是完整路径
        // 检查文件是否存在
        if(!fs::exists(path)) {
            spdlog::error("加载模块时出错，没有找到文件");
            JS_ThrowReferenceError(m_ctx, "没有找到模块: %s", path.c_str());
            return nullptr;
        }

        // 将相对路径变为绝对路径
        //std::filesystem::path _path = std::filesystem::weakly_canonical(std::string(path));
        //std::filesystem::path p0 = std::filesystem::canonical(std::filesystem::path(path));
        //std::string path2 = p0.string();
        //spdlog::info("{}", path2);


        // 加载模块
        std::ifstream jsfile(path);
        std::string content((std::istreambuf_iterator<char>(jsfile)), std::istreambuf_iterator<char>());
        
        std::size_t hasx = std::hash<std::string>()(content);
        auto it = onlyLoad.find(hasx);
        if(it != onlyLoad.end()) {
            jsfile.close();
            return it->second;
        }

        JSValue val = JS_Eval(m_ctx, content.c_str(), content.size(), path.c_str(), JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY/* | JS_EVAL_FLAG_ASYNC 只允许在 JS_EVAL_TYPE_GLOBAL 中使用*//* | JS_EVAL_TYPE_MASK*/);
        if(JS_IsException(val)) {
            spdlog::error("JS Module Loader fail: {}", path);
            auto result_str = JS_ErrorStackCheck(m_ctx);
            spdlog::error(result_str);
            jsfile.close();
            return nullptr;
        }
        js_module_set_import_meta(m_ctx, val, true, true);
        //val = JS_EvalFunction(m_ctx, val);
        JSModuleDef* module = (JSModuleDef*)JS_VALUE_GET_PTR(val);
        onlyLoad[hasx] = module;
        JS_FreeValue(m_ctx, val);
        jsfile.close();
        return module;
    }
    catch(std::exception& e) {
        spdlog::error("加载模块时出现错误:{} of {} in {}",e.what(), __FUNCTION__, __LINE__);
        return nullptr;
    }
}

auto JSManager::loadModuleFromHttp(const std::string& url) -> JSModuleDef* {
    static std::unordered_map<std::size_t, JSModuleDef*> onlyLoad{};
    try {

        std::size_t hasx = std::hash<std::string>()(url); // 仅从url判断此库是否被加载过了, 避免重复的网络请求
        // 从map缓存中找 是否被加载过了
        auto it = onlyLoad.find(hasx);
        if(it != onlyLoad.end()) {
            return it->second;
        }

        // 网络协议开头
        std::regex pattern(
            R"(^(?:(\w+):\/\/)?)"    // 协议 (可选)
            R"(([^/?#:]+(:\d+)?))"   // 主机+端口 (必需)
            R"((\/[^ ]*)?$)"          // 路径 (可选)
        );
        std::smatch match;
        if(std::regex_match(url, match, pattern)) {
            std::string host_port = match[2].str();
            std::string path = match[4].matched ? match[4].str() : "/";

            httplib::Client http(host_port);
            if(!http.is_valid()) {
                spdlog::error("载入模块[{}]时,网址的链接无法解析主机和端口", url.c_str());
                JS_ThrowReferenceError(m_ctx, "无法解析模块主机和端口");
                return nullptr;
            }
            http.set_follow_location(true);
            http.enable_server_certificate_verification(false);
            auto ret = http.Get(path);
            if(ret->status != httplib::StatusCode::OK_200) {
                spdlog::error("载入模块[{}]时,链接请求结果为:{}", url.c_str(), ret->status);
                JS_ThrowReferenceError(m_ctx, "远程模块请求结尾非为200");
                return nullptr;
            }
            std::string content = ret->body;

            JSValue val = JS_Eval(m_ctx, content.c_str(), content.size(), path.c_str(), JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY/* | JS_EVAL_FLAG_ASYNC 只允许在 JS_EVAL_TYPE_GLOBAL 中使用*/  /* | JS_EVAL_TYPE_MASK*/);
            if(JS_IsException(val)) {
                spdlog::error("JS Module Loader fail: {}", path);
                auto result_str = JS_ErrorStackCheck(m_ctx);
                spdlog::error(result_str);
                return nullptr;
            }
            js_module_set_import_meta(m_ctx, val, true, true);
            JSModuleDef* module = (JSModuleDef*)JS_VALUE_GET_PTR(val);
            onlyLoad[hasx] = module;
            JS_FreeValue(m_ctx, val);
            return module;
        }
        spdlog::error("载入模块[{}]时, 正则匹配此网址失败", url.c_str());
        JS_ThrowReferenceError(m_ctx, "正则匹配网址失败");
        return nullptr;
    }
    catch(std::exception& e) {
        spdlog::error("加载模块时出现错误:{} of {} in {}", e.what(), __FUNCTION__, __LINE__);
        return nullptr;
    }
}

// BaseFolder script, load All File
auto JSManager::loadJSFromFoder(const std::string& folder) -> void {
    for(const auto& entry : fs::directory_iterator(ModManager::getInstance()->getPath("script") / folder)) {
        if(entry.is_regular_file() && entry.path().extension() == ".js") {
            try {
                std::ifstream jsfile(entry.path());
                std::string content((std::istreambuf_iterator<char>(jsfile)), std::istreambuf_iterator<char>());
                spdlog::info("JS Loader: {}", entry.path().filename().string());
                std::string scriptname = entry.path().filename().string();
                // 新建一个JS对象
                auto gobj = JS_GetGlobalObject(m_ctx);
                onJsLoadBefore(scriptname, gobj);

                JSValue val = JS_EvalThis(m_ctx, gobj, content.c_str(), content.size(), (fs::path(folder) / entry.path().filename()).string().c_str(), JS_EVAL_TYPE_MODULE/* | JS_EVAL_FLAG_ASYNC 只允许在 JS_EVAL_TYPE_GLOBAL 中使用*/);
                onJsLoadAfter(scriptname, gobj);
                if(JS_IsException(val)) {
                    spdlog::error("JS Loader fail: {}", entry.path().string());
                    auto result_str = JS_ErrorStackCheck(m_ctx);
                    spdlog::error(result_str);
                }

                JS_FreeValue(m_ctx, val);
                JS_FreeValue(m_ctx, gobj);
            }
            catch(std::exception& ex) {
                spdlog::error("加载{}时出现异常：{}", entry.path().filename().string().c_str(), ex.what());
            }
        }
    }
    loadConfig();
}

static JSValue JSImConsoleWindow(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        // 返回状态
        return JS_NewBool(ctx, GetImguiConsole()->MainOpen);
    }
    if(!JS_IsBool(argv[0])) {
        return JS_ThrowTypeError(ctx, "只能接受一个bool类型参数");
    }
    GetImguiConsole()->MainOpen = (bool)JS_ToBool(ctx, argv[0]);
    return JS_UNDEFINED;
}



std::unordered_map<std::string, JSValue> modulesig_map;
static std::shared_mutex rw_mtx_modulesigList;

// 在JS执行之前处理this对象 写入环境信息
auto JSManager::onJsLoadBefore(const std::string name, JSValue& jsv) -> void {
    JSValue sigobj = JS_NewObject(m_ctx);
    JS_SetPropertyStr(m_ctx, sigobj, "enable", JS_NewBool(m_ctx, true));
    JS_SetPropertyStr(m_ctx, sigobj, "name", JS_NewString(m_ctx, name.c_str()));
    JS_SetPropertyStr(m_ctx, sigobj, "JSConsoleWindow", JS_NewCFunction(m_ctx, JSImConsoleWindow, "JSConsoleWindow", 1));
    JS_SetPropertyStr(m_ctx, jsv, "__模块__", sigobj);
    std::unique_lock<std::shared_mutex> lock(rw_mtx_modulesigList);
    modulesig_map[name] = JS_DupValue(m_ctx, sigobj);
    //JS_FreeValue(m_ctx, sigobj);
}

// 在JS执行之后处理this对象 比如拿到某某函数再执行
auto JSManager::onJsLoadAfter(const std::string name, JSValue& jsv) -> void {
    // 给这个对象写入属性
    JS_SetPropertyStr(m_ctx, jsv, "__模块__", JS_NULL);
}

auto JSManager::runstdLoop() -> bool {
    bool ret = false;
    JSContext* ctx1;

    int v = JS_ExecutePendingJob(JS_GetRuntime(m_ctx), &ctx1);
    if(v < 0) {
       int err = JS_HasException(m_ctx);
       spdlog::error("std_loop ctx error: {}", this->getErrorStack().c_str());
       ret = err;
    }

    //if(ret = js_std_loop(m_ctx)) {
    //    spdlog::error("std_loop ctx error: {}", this->getErrorStack().c_str());
    //}
    return ret;
}

auto JSManager::getErrorStack(JSValue err) -> std::string {
    bool isError = JS_IsError(m_ctx, err);
    std::string result_str;
    if(isError) {
        JSValue name = JS_GetPropertyStr(m_ctx, err, "name");
        const char* errorname_str = JS_ToCString(m_ctx, name);
        result_str = (errorname_str) ? errorname_str : "<unknown error name>";
        result_str += " - ";
        JS_FreeCString(m_ctx, errorname_str);
        JS_FreeValue(m_ctx, name);
        JSValue message = JS_GetPropertyStr(m_ctx, err, "message");
        const char* message_str = JS_ToCString(m_ctx, message);
        result_str += (message_str) ? std::string("\"") + message_str + "\"" : "<no message>";
        result_str += ":: \n";
        JS_FreeCString(m_ctx, message_str);
        JS_FreeValue(m_ctx, message);
        JSValue stack = JS_GetPropertyStr(m_ctx, err, "stack");
        if(!JS_IsUndefined(stack)) {
            const char* stack_str = JS_ToCString(m_ctx, stack);
            if(stack_str) {
                result_str += stack_str;
                JS_FreeCString(m_ctx, stack_str);
            }
        }
        JS_FreeValue(m_ctx, stack);
    }
    return result_str;
}

auto JSManager::getErrorStack() -> std::string {
    auto jsv = JS_GetException(m_ctx);
    auto err = getErrorStack(jsv);
    JS_FreeValue(this->getctx(), jsv);
    return err;
}

auto JSManager::loadConfig() -> void {
    auto g = JS_GetGlobalObject(m_ctx);
    std::string config_str = ConfigManager::getInstance()->asString();
    if(config_str.empty()) {
        spdlog::warn("读取配置失败, 配置为空");
    }
    else {
        JSValue JSconfig = JS_ParseJSON(m_ctx, config_str.c_str(), config_str.size(), ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
        if(JS_IsException(JSconfig)) {
            spdlog::error("读取配置文件解析JSON时出现错误 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
        }
        else {
            JSValue v[] = { JSconfig };
            // 广播事件
            NativeBroadcastEvent("onLoad", g, 1, v);
        }
        JS_FreeValue(m_ctx, JSconfig);
    }
    JS_FreeValue(m_ctx, g);

    //std::string config_str = "";
    //std::ifstream configFileR(ModManager::getInstance()->getOtherPath("ModConfig"), std::ios::in | std::ios::out);
    //if(!configFileR.is_open()) {
    //    spdlog::error("读取配置文件失败, 配置文件无法打开 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
    //}
    //else {
    //    config_str = std::string(std::istreambuf_iterator<char>(configFileR), std::istreambuf_iterator<char>());
    //    configFileR.close();
    //    JSValue JSconfig = JS_ParseJSON(m_ctx, config_str.c_str(), config_str.size(), ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
    //    if(JS_IsException(JSconfig)) {
    //        spdlog::error("读取配置文件解析JSON时出现错误 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
    //    }
    //    else {
    //        JSValue v[] = { JSconfig };
    //        // 广播事件
    //        NativeBroadcastEvent("onLoad", g, 1, v);
    //    }
    //    JS_FreeValue(m_ctx, JSconfig);
    //}
    //JS_FreeValue(m_ctx, g);
}
auto JSManager::saveConfig() -> void {
    auto g = JS_GetGlobalObject(m_ctx);
    std::string config_str = ConfigManager::getInstance()->asString();
    if(config_str.empty()) {
        spdlog::warn("读取配置失败, 配置为空");
    }
    else {
        JSValue JSconfig = JS_ParseJSON(m_ctx, config_str.c_str(), config_str.size(), ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
        if(JS_IsException(JSconfig)) {
            spdlog::error("读取配置文件解析JSON时出现错误 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
        }
        else {
            JSValue v[] = { JSconfig };
            // 广播事件
            NativeBroadcastEvent("onSave", g, 1, v);
            // 重新写回
            JSValue json_str = JS_JSONStringify(m_ctx, JSconfig, JS_NULL, JS_NewInt32(m_ctx, 2));
            if(JS_IsException(json_str)) {
                spdlog::error("将脚本中的配置信息重新序列化为字符串的时候出错 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
            }
            else {
                auto newString = JSTool::toString(json_str);
                if(!newString) {
                    spdlog::error("将脚本中的配置信息重新序列化为字符串的时候出错 .2 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
                }
                else {
                    std::ofstream configFileW(ModManager::getInstance()->getOtherPath("ModConfig"), std::ios::out | std::ios::trunc);
                    if(!configFileW.is_open()) {
                        spdlog::error("在打开配置文件(准备写入)时失败 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
                    }
                    else {
                        configFileW.write(newString->c_str(), newString->size());
                        configFileW.close();
                        spdlog::info("配置保存成功, 写入 {} 字节", newString->size());
                    }
                }
            }
            JS_FreeValue(m_ctx, json_str);
        }
        JS_FreeValue(m_ctx, JSconfig);
    }
    JS_FreeValue(m_ctx, g);



    //std::string config_str = "";
    //std::ifstream configFileR(ModManager::getInstance()->getOtherPath("ModConfig"), std::ios::in | std::ios::out);
    //if(!configFileR.is_open()) {
    //    spdlog::error("保存失败, 配置文件无法打开 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
    //}
    //else {
    //    config_str = std::string(std::istreambuf_iterator<char>(configFileR), std::istreambuf_iterator<char>());
    //    configFileR.close();

    //    JSValue JSconfig = JS_ParseJSON(m_ctx, config_str.c_str(), config_str.size(), ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
    //    if(JS_IsException(JSconfig)) {
    //        spdlog::error("读取配置文件解析JSON时出现错误 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
    //    }
    //    else {
    //        JSValue v[] = { JSconfig };
    //        // 广播事件
    //        NativeBroadcastEvent("onSave", g, 1, v);
    //        // 重新写回
    //        JSValue json_str = JS_JSONStringify(m_ctx, JSconfig, JS_NULL, JS_NewInt32(m_ctx, 2));
    //        if(JS_IsException(json_str)) {
    //            spdlog::error("将脚本中的配置信息重新序列化为字符串的时候出错 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
    //        }
    //        else {
    //            auto newString = JSTool::toString(json_str);
    //            if(!newString) {
    //                spdlog::error("将脚本中的配置信息重新序列化为字符串的时候出错 .2 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
    //            }
    //            else {
    //                std::ofstream configFileW(ModManager::getInstance()->getOtherPath("ModConfig"), std::ios::out | std::ios::trunc);
    //                if(!configFileW.is_open()) {
    //                    spdlog::error("在打开配置文件(准备写入)时失败 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, ModManager::getInstance()->getOtherPath("ModConfig").string().c_str());
    //                }
    //                else {
    //                    configFileW.write(newString->c_str(), newString->size());
    //                    configFileW.close();
    //                    spdlog::info("配置保存成功, 写入 {} 字节", newString->size());
    //                }
    //            }
    //        }
    //        JS_FreeValue(m_ctx, json_str);
    //    }
    //    JS_FreeValue(m_ctx, JSconfig);
    //}
    //JS_FreeValue(m_ctx, g);
}


auto JSManager::onImGuiRenderScriptSig() -> void {

    if(ImGui::Button("读取")) {
        loadConfig();
    }

    ImGui::SameLine();
    if(ImGui::Button("保存")) {
        saveConfig();
    }

    std::shared_lock<std::shared_mutex> lock(rw_mtx_modulesigList);
    for(auto& kv : modulesig_map) {
        bool open = true;
        JSTool::ReferenceValue(&open, kv.second, "enable");
        if(ImGui::Checkbox(kv.first.c_str(), &open)){
            JSTool::ReferenceValue(open, kv.second, "enable");
        }
    }

}

auto JSManager::onImGuiRender() -> void {
    NativeBroadcastEvent("onRender");
}


auto JSManager::initJSManager()->void {
    
}

//void unregisterCoreWindowEventHandle();
auto JSManager::disableJSManager() -> void {

}


auto JSManager::runTaskQueue() -> bool {
    // 线程锁
    std::lock_guard<std::recursive_mutex> lock(taskQueueMtx);

    // 获取当前时间点（兼容 Windows/Linux/macOS）
    auto now = std::chrono::system_clock::now();
    // 转换为毫秒级时间戳
    auto ms_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();

    for(auto it = TaskQueues.begin(); it != TaskQueues.end(); ) {
        if(it->second.curr_t + it->second.time < ms_since_epoch) {
            try {
                JSValue ret = JS_Call(m_ctx, it->second.call, JS_NULL, 0, {});
                if(JS_IsException(ret)) {
                    spdlog::error(this->getErrorStack(ret));
                }
                JS_FreeValue(m_ctx, ret);
            }
            catch(std::exception& e) {
                spdlog::error("执行任务队列回调时出现异常 {}, {}, {}", e.what(), __FILE__, __LINE__);
            }
            if(it->second.type == std::byte(0)) {
                JS_FreeValue(m_ctx, it->second.call);
                freeKeys.push(it->first);
                it = TaskQueues.erase(it); // C++11 起 erase 返回下一迭代器
            }
            else { // == 1
                it->second.curr_t = ms_since_epoch;
                ++it;
            }
        }
        else {
            //it->second.curr_t = ms_since_epoch;
            ++it;
        }
    }

    return TaskQueues.size() > 0;
}

auto JSManager::addTimeOut(JSValue jsv, int64_t t) -> int64_t {
    // 线程锁
    std::lock_guard<std::recursive_mutex> lock(taskQueueMtx);

    // 获取当前时间点（兼容 Windows/Linux/macOS）
    auto now = std::chrono::system_clock::now();
    // 转换为毫秒级时间戳
    auto ms_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();

    int64_t key = 0;
    if(!freeKeys.empty()) {
        key = freeKeys.top();
        freeKeys.pop();
    }
    else {
        key = nextKey++; // 分配新键
    }
    TaskQueues[key] = {
        JS_DupValue(m_ctx, jsv),
        std::byte(0),
        t,
        ms_since_epoch
    };

    return key;
}

auto JSManager::addInterval(JSValue jsv, int64_t t) -> int64_t {
    // 线程锁
    std::lock_guard<std::recursive_mutex> lock(taskQueueMtx);

    // 获取当前时间点（兼容 Windows/Linux/macOS）
    auto now = std::chrono::system_clock::now();
    // 转换为毫秒级时间戳
    auto ms_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();

    int64_t key = 0;
    if(!freeKeys.empty()) {
        key = freeKeys.top();
        freeKeys.pop();
    }
    else {
        key = nextKey++; // 分配新键
    }
    TaskQueues[key] = {
        JS_DupValue(m_ctx, jsv),
        std::byte(1),
        t,
        ms_since_epoch
    };

    return key;
}

auto JSManager::removeTask(int64_t id) -> void {
    // 线程锁
    std::lock_guard<std::recursive_mutex> lock(taskQueueMtx);
    auto it = TaskQueues.find(id);
    if(it != TaskQueues.end()) {
        JS_FreeValue(m_ctx, it->second.call);
        TaskQueues.erase(it); // 按迭代器删除
        freeKeys.push(id);
    }
}



//////////////////////// JSTool ////////////////////////

std::optional<std::string> JSTool::toString(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<std::string> str;
    if(!JS_IsString(jsv)) {
        jsv = JS_ToString(ctx, jsv);
    }
    if(JS_IsString(jsv)) {
        const char* s = JS_ToCString(ctx, jsv);
        str = std::string(s);
        JS_FreeCString(ctx, s);
        return str;
    }
    return str;
}

std::optional<int32_t> JSTool::toInt(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<int32_t> intval;
    INT32 value;
    if(!JS_IsNumber(jsv) || JS_ToInt32(ctx, &value, jsv) < 0) {
        return intval;
    }
    intval = value;
    return intval;
}

std::optional<int64_t> JSTool::toInt64(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<int64_t> intval;
    INT64 value;
    if(!JS_IsNumber(jsv) || JS_ToInt64(ctx, &value, jsv) < 0) {
        return intval;
    }
    intval = value;
    return intval;
}

std::optional<float> JSTool::toFloat(JSValue jsv) {
    std::optional<float> fval;
    auto value = JSTool::toDouble(jsv);
    if(value) {
        if(*value > std::numeric_limits<float>::max()) {
            return fval;
        }
        fval = static_cast<float>(value.value());
    }
    return fval;
}

std::optional<double> JSTool::toDouble(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<double> fval;
    double value;
    if(!JS_IsNumber(jsv) || JS_ToFloat64(ctx, &value, jsv) < 0) {
        return fval;
    }
    fval = value;
    return fval;
}

std::optional<std::array<float,2>> JSTool::getPropXY(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<std::array<float,2>> ret;
    JSValue xValue = JS_GetPropertyStr(ctx, jsv, "x");
    JSValue yValue = JS_GetPropertyStr(ctx, jsv, "y");
    double x, y;
    if(!JS_IsNumber(xValue) || !JS_IsNumber(yValue) || JS_ToFloat64(ctx, &x, xValue) < 0 || JS_ToFloat64(ctx, &y, yValue) < 0) {
        return ret;
    }
    ret = { static_cast<float>(x), static_cast<float>(y) };
    return ret;
}

std::optional<std::array<float, 3>> JSTool::getPropXYZ(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<std::array<float, 3>> ret;
    JSValue xValue = JS_GetPropertyStr(ctx, jsv, "x");
    JSValue yValue = JS_GetPropertyStr(ctx, jsv, "y");
    JSValue zValue = JS_GetPropertyStr(ctx, jsv, "z");
    double x, y, z;
    if(!JS_IsNumber(xValue) || !JS_IsNumber(yValue) || !JS_IsNumber(zValue) || JS_ToFloat64(ctx, &x, xValue) < 0 || JS_ToFloat64(ctx, &y, yValue) < 0 || JS_ToFloat64(ctx, &z, zValue) < 0) {
        return ret;
    }
    ret = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) };
    return ret;
}

std::optional<std::array<float, 3>> JSTool::getPropRGB(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<std::array<float, 3>> ret;
    JSValue xValue = JS_GetPropertyStr(ctx, jsv, "r");
    JSValue yValue = JS_GetPropertyStr(ctx, jsv, "g");
    JSValue zValue = JS_GetPropertyStr(ctx, jsv, "b");
    double x, y, z;
    if(!JS_IsNumber(xValue) || !JS_IsNumber(yValue) || !JS_IsNumber(zValue) || JS_ToFloat64(ctx, &x, xValue) < 0 || JS_ToFloat64(ctx, &y, yValue) < 0 || JS_ToFloat64(ctx, &z, zValue) < 0) {
        return ret;
    }
    ret = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) };
    return ret;
}

std::optional<std::array<float, 4>> JSTool::getPropXYZW(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<std::array<float, 4>> ret;
    JSValue xValue = JS_GetPropertyStr(ctx, jsv, "x");
    JSValue yValue = JS_GetPropertyStr(ctx, jsv, "y");
    JSValue zValue = JS_GetPropertyStr(ctx, jsv, "z");
    JSValue wValue = JS_GetPropertyStr(ctx, jsv, "w");
    double x, y, z, w;
    if(!JS_IsNumber(xValue) || !JS_IsNumber(yValue) || !JS_IsNumber(zValue) || !JS_IsNumber(wValue) 
       || JS_ToFloat64(ctx, &x, xValue) < 0 || JS_ToFloat64(ctx, &y, yValue) < 0 || JS_ToFloat64(ctx, &z, zValue) < 0 || JS_ToFloat64(ctx, &w, wValue) < 0) {
        return ret;
    }
    ret = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w) };
    return ret;
}

std::optional<std::array<float, 4>> JSTool::getPropRGBA(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<std::array<float, 4>> ret;
    JSValue xValue = JS_GetPropertyStr(ctx, jsv, "r");
    JSValue yValue = JS_GetPropertyStr(ctx, jsv, "g");
    JSValue zValue = JS_GetPropertyStr(ctx, jsv, "b");
    JSValue wValue = JS_GetPropertyStr(ctx, jsv, "a");
    double x, y, z, w;
    if(!JS_IsNumber(xValue) || !JS_IsNumber(yValue) || !JS_IsNumber(zValue) || !JS_IsNumber(wValue)
       || JS_ToFloat64(ctx, &x, xValue) < 0 || JS_ToFloat64(ctx, &y, yValue) < 0 || JS_ToFloat64(ctx, &z, zValue) < 0 || JS_ToFloat64(ctx, &w, wValue) < 0) {
        return ret;
    }
    ret = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w) };
    return ret;
}

bool JSTool::setPropXY(JSValue jsv, std::array<float, 2> v) {
    auto ctx = JSManager::getInstance()->getctx();
    auto x = JS_NewFloat64(ctx, v.at(0));
    auto y = JS_NewFloat64(ctx, v.at(1));

    if(JS_SetPropertyStr(ctx, jsv, "x", x) < 0) return false;
    if(JS_SetPropertyStr(ctx, jsv, "y", y) < 0) return false;

    JS_FreeValue(ctx, x);
    JS_FreeValue(ctx, y);

    return true;
}

bool JSTool::setPropXYZ(JSValue jsv, std::array<float, 3> v) {
    auto ctx = JSManager::getInstance()->getctx();
    auto x = JS_NewFloat64(ctx, v.at(0));
    auto y = JS_NewFloat64(ctx, v.at(1));
    auto z = JS_NewFloat64(ctx, v.at(2));

    if(JS_SetPropertyStr(ctx, jsv, "x", x) < 0) return false;
    if(JS_SetPropertyStr(ctx, jsv, "y", y) < 0) return false;
    if(JS_SetPropertyStr(ctx, jsv, "z", z) < 0) return false;

    JS_FreeValue(ctx, x);
    JS_FreeValue(ctx, y);
    JS_FreeValue(ctx, z);

    return true;
}

bool JSTool::setPropXYZW(JSValue jsv, std::array<float, 4> v) {
    auto ctx = JSManager::getInstance()->getctx();
    auto x = JS_NewFloat64(ctx, v.at(0));
    auto y = JS_NewFloat64(ctx, v.at(1));
    auto z = JS_NewFloat64(ctx, v.at(2));
    auto w = JS_NewFloat64(ctx, v.at(3));

    if(JS_SetPropertyStr(ctx, jsv, "x", x) < 0) return false;
    if(JS_SetPropertyStr(ctx, jsv, "y", y) < 0) return false;
    if(JS_SetPropertyStr(ctx, jsv, "z", z) < 0) return false;
    if(JS_SetPropertyStr(ctx, jsv, "w", w) < 0) return false;

    JS_FreeValue(ctx, x);
    JS_FreeValue(ctx, y);
    JS_FreeValue(ctx, z);
    JS_FreeValue(ctx, w);

    return true;
}

bool JSTool::setPropRGB(JSValue jsv, std::array<float, 3> v) {
    auto ctx = JSManager::getInstance()->getctx();
    auto x = JS_NewFloat64(ctx, v.at(0));
    auto y = JS_NewFloat64(ctx, v.at(1));
    auto z = JS_NewFloat64(ctx, v.at(2));

    if(JS_SetPropertyStr(ctx, jsv, "r", x) < 0) return false;
    if(JS_SetPropertyStr(ctx, jsv, "g", y) < 0) return false;
    if(JS_SetPropertyStr(ctx, jsv, "b", z) < 0) return false;

    JS_FreeValue(ctx, x);
    JS_FreeValue(ctx, y);
    JS_FreeValue(ctx, z);

    return true;
}

bool JSTool::setPropRGBA(JSValue jsv, std::array<float, 4> v) {
    auto ctx = JSManager::getInstance()->getctx();
    auto x = JS_NewFloat64(ctx, v.at(0));
    auto y = JS_NewFloat64(ctx, v.at(1));
    auto z = JS_NewFloat64(ctx, v.at(2));
    auto w = JS_NewFloat64(ctx, v.at(3));

    if(JS_SetPropertyStr(ctx, jsv, "r", x) < 0) return false;
    if(JS_SetPropertyStr(ctx, jsv, "g", y) < 0) return false;
    if(JS_SetPropertyStr(ctx, jsv, "b", z) < 0) return false;
    if(JS_SetPropertyStr(ctx, jsv, "a", w) < 0) return false;

    JS_FreeValue(ctx, x);
    JS_FreeValue(ctx, y);
    JS_FreeValue(ctx, z);
    JS_FreeValue(ctx, w);

    return true;
}

int JSTool::setPropFunc(JSValue obj, JSCFunction* fun, std::string str, int length) {
    auto ctx = JSManager::getInstance()->getctx();
    JSValue jsfun = JS_NewCFunction(ctx, fun, str.c_str(), length);
    int retv = JS_SetPropertyStr(ctx, obj, str.c_str(), jsfun);
    //JS_FreeValue(ctx, jsfun);
    return retv;
}

JSValue JSTool::getConstructorValue(JSValue thisObj, JSClassID id, bool inheritance) {
    auto ctx = JSManager::getInstance()->getctx();
    if(!inheritance) {   // 如果不允许被继承
        return JS_NewObjectClass(ctx, id);
    }
    JSValue proto = JS_GetPropertyStr(ctx, thisObj, "prototype");
    if(JS_IsException(proto) || JS_IsError(ctx, proto) || JS_IsUndefined(proto)) {
        return JS_NewObjectClass(ctx, id);
    }
    JSValue obj = JS_NewObjectProtoClass(ctx, proto, id);
    JS_FreeValue(ctx, proto);
    return obj;
}

std::optional<std::vector<JSValue>> JSTool::toArray(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<std::vector<JSValue>> ret{};
    if(!JS_IsArray(jsv)) {
        return ret;
    }
    std::vector<JSValue> v{};
    JSValue lengthVal = JS_GetPropertyStr(ctx, jsv, "length");
    uint64_t len = 0;
    JS_ToIndex(ctx, &len, lengthVal);
    JS_FreeValue(ctx, lengthVal);
    for(int i = 0; i < len; i++) {
        JSValue item = JS_GetPropertyUint32(ctx, jsv, i);
        v.push_back(item);
    }
    ret = v;
    return ret;
}
#include <cctype>
#include <algorithm>
std::optional<std::vector<std::pair<std::string, std::string>>> JSTool::getObjKV(JSValue obj) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<std::vector<std::pair<std::string, std::string>>> ret;
    std::vector<std::pair<std::string, std::string>> allItem = {};

    // 1. 检查是否为对象
    if(!JS_IsObject(obj)) {
        return ret;
    }

    // 2. 获取属性列表
    JSPropertyEnum* props;
    uint32_t count;
    int flags = JS_GPN_STRING_MASK | JS_GPN_ENUM_ONLY;
    if(JS_GetOwnPropertyNames(ctx, &props, &count, obj, flags)) {
        return ret;
    }

    // 3. 遍历所有属性
    for(uint32_t i = 0; i < count; i++) {
        // 获取属性名（键）
        JSAtom atom = props[i].atom;
        JSValue keyVal = JS_AtomToString(ctx, atom);
        const char* keyStr = JS_ToCString(ctx, keyVal);
        std::string key = keyStr ? keyStr : "";

        // 获取属性值
        JSValue val = JS_GetProperty(ctx, obj, atom);
        auto value = JSTool::toString(val);
        if(!value) return ret;

        // 释放资源
        JS_FreeCString(ctx, keyStr);
        JS_FreeValue(ctx, keyVal);
        JS_FreeValue(ctx, val);

        // 存入结果
        if(!key.empty()) {
            allItem.push_back({ key , *value });
        }
    }
    // 4. 释放属性列表
    
    uint32_t i;
    if(props) {
        for(i = 0; i < count; i++)
            JS_FreeAtom(ctx, props[i].atom);
        js_free(ctx, props);
    }
    ret = allItem;
    return ret;
}

JSValue JSTool::fromString(const char* str) {
    auto ctx = JSManager::getInstance()->getctx();
    JSValue js_str = JS_NewString(ctx, str);

    if(JS_IsException(js_str)) {
        // 处理异常
        JS_FreeValue(ctx, js_str);
        return JS_EXCEPTION;
    }
    return js_str;
}

JSValue JSTool::fromString(std::string str) {
    auto ctx = JSManager::getInstance()->getctx();
    JSValue js_str = JS_NewString(ctx, str.c_str());

    if(JS_IsException(js_str)) {
        // 处理异常
        JS_FreeValue(ctx, js_str);
        return JS_EXCEPTION;
    }
    return js_str;
}

bool JSTool::isFun(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    return JS_IsFunction(ctx, jsv);
}




JSValue JSTool::ReferenceValue(bool* v, JSValue jsv, const char* name) {
    auto ctx = JSManager::getInstance()->getctx();
    std::string pname = name ? name : "bool";
    if(!JS_IsObject(jsv)) {
        return JS_ThrowTypeError(ctx, "引用值非对象,确保传入一个对象(修改属性[%s])", pname.c_str());
    }
    auto obj = JS_GetPropertyStr(ctx, jsv, pname.c_str());
    if(JS_IsUndefined(obj)) {
        return JS_ThrowTypeError(ctx, "引用值没有此属性(%s)", pname.c_str());
    }
    if(JS_IsBool(obj) && JS_ToBool(ctx, obj)) {
        *v = true;
    }
    else {
        *v = false;
    }
    return JS_UNDEFINED;
    //JS_SetPropertyStr(ctx, argv[0], "bool", JS_NewBool(ctx, show));
}

void JSTool::ReferenceValue(bool v, JSValue jsv, const char* name) {
    auto ctx = JSManager::getInstance()->getctx();
    std::string pname = name ? name : "bool";
    auto obj = JS_GetPropertyStr(ctx, jsv, pname.c_str());
    JS_SetPropertyStr(ctx, jsv, pname.c_str(), JS_NewBool(ctx, v));
}

JSValue JSTool::ReferenceValue(int* v, JSValue jsv, const char* name) {
    auto ctx = JSManager::getInstance()->getctx();
    std::string pname = name ? name : "int";
    if(!JS_IsObject(jsv)) {
        return JS_ThrowTypeError(ctx, "引用值非对象,确保传入一个对象(修改属性[%s])", pname.c_str());
    }
    auto obj = JS_GetPropertyStr(ctx, jsv, pname.c_str());
    if(JS_IsUndefined(obj)) {
        return JS_ThrowTypeError(ctx, "引用值没有此属性(%s)", pname.c_str());
    }
    if(!JS_IsNumber(obj) || JS_ToInt32(ctx, v, obj) < 0) {
        return JS_ThrowTypeError(ctx, "引用值属性(%s)不是一个合法的Number", pname.c_str());
    }
    return JS_UNDEFINED;
    //JS_SetPropertyStr(ctx, argv[0], "int", JS_NewBool(ctx, show));
}

void JSTool::ReferenceValue(int v, JSValue jsv, const char* name) {
    auto ctx = JSManager::getInstance()->getctx();
    std::string pname = name ? name : "int";
    auto obj = JS_GetPropertyStr(ctx, jsv, pname.c_str());
    JS_SetPropertyStr(ctx, jsv, pname.c_str(), JS_NewInt32(ctx, v));
}

JSValue JSTool::ReferenceValue(double* v, JSValue jsv, const char* name) {
    auto ctx = JSManager::getInstance()->getctx();
    std::string pname = name ? name : "double";
    if(!JS_IsObject(jsv)) {
        return JS_ThrowTypeError(ctx, "引用值非对象,确保传入一个对象(修改属性[%s])", pname.c_str());
    }
    auto obj = JS_GetPropertyStr(ctx, jsv, pname.c_str());
    if(JS_IsUndefined(obj)) {
        return JS_ThrowTypeError(ctx, "引用值没有此属性(%s)", pname.c_str());
    }
    if(!JS_IsNumber(obj) || JS_ToFloat64(ctx, v, obj) < 0) {
        return JS_ThrowTypeError(ctx, "引用值属性(%s)不是一个合法的Number", pname.c_str());
    }
    return JS_UNDEFINED;
    //JS_SetPropertyStr(ctx, argv[0], "int", JS_NewBool(ctx, show));
}

void JSTool::ReferenceValue(double v, JSValue jsv, const char* name) {
    auto ctx = JSManager::getInstance()->getctx();
    std::string pname = name ? name : "double";
    auto obj = JS_GetPropertyStr(ctx, jsv, pname.c_str());
    JS_SetPropertyStr(ctx, jsv, pname.c_str(), JS_NewFloat64(ctx, v));
}

JSValue JSTool::ReferenceValue(float* v, JSValue jsv, const char* name) {
    auto ctx = JSManager::getInstance()->getctx();
    std::string pname = name ? name : "float";
    return JSTool::ReferenceValue(reinterpret_cast<double*>(v), jsv, pname.c_str());
}

void JSTool::ReferenceValue(float v, JSValue jsv, const char* name) {
    auto ctx = JSManager::getInstance()->getctx();
    std::string pname = name ? name : "float";
    JSTool::ReferenceValue(static_cast<double>(v), jsv, pname.c_str());
}

std::string JSTool::ReferenceBoolCall(JSValue jsv, std::function<void(bool*)> call) {
    try {
        auto ctx = JSManager::getInstance()->getctx();
        bool show = false;
        if(!JS_IsFunction(ctx, jsv)) {
            return "传入的参数不是一个回调函数";
        }
        JSValue callret = JS_Call(ctx, jsv, JS_NULL, 0, nullptr);   // 首先调用一个空参获取一个值
        if(JS_IsException(callret)) {
            JS_FreeValue(ctx, callret);
            return "JS函数调用抛出异常";
        }
        int retv = JS_ToBool(ctx, callret);
        JS_FreeValue(ctx, callret);
        if(retv < 0) {
            return "Call的返回值转为bool时失败";
        }
        show = retv;
        call(&show);

        JSValue canshuv = JS_NewBool(ctx, show);
        JSValue canshu[] = { canshuv };
        JS_FreeValue(ctx, JS_Call(ctx, jsv, JS_NULL, 1, canshu));
        JS_FreeValue(ctx, canshuv);
    }
    catch(std::exception& ex) {
        return ex.what();
    }
    return std::string();
}

std::string JSTool::ReferenceIntCall(JSValue jsv, std::function<void(int*)> call) {
    try {
        auto ctx = JSManager::getInstance()->getctx();
        int v = 0;
        if(!JS_IsFunction(ctx, jsv)) {
            return "传入的参数不是一个回调函数";
        }
        JSValue callret = JS_Call(ctx, jsv, JS_NULL, 0, nullptr);   // 首先调用一个空参获取一个值
        if(JS_IsException(callret)) {
            JS_FreeValue(ctx, callret);
            return "JS函数调用抛出异常";
        }
        auto retv = toInt(callret);
        JS_FreeValue(ctx, callret);
        if(!retv) {
            return "Call的返回值转为int时失败";
        }
        v = *retv;
        call(&v);

        JSValue canshuv = JS_NewInt32(ctx, v);
        JSValue canshu[] = { canshuv };
        JS_FreeValue(ctx, JS_Call(ctx, jsv, JS_NULL, 1, canshu));
        JS_FreeValue(ctx, canshuv);
    }
    catch(std::exception& ex) {
        return ex.what();
    }
    return std::string();
}

std::string JSTool::ReferenceFloatCall(JSValue jsv, std::function<void(float*)> call) {
    try {
        auto ctx = JSManager::getInstance()->getctx();
        float v = 0;
        if(!JS_IsFunction(ctx, jsv)) {
            return "传入的参数不是一个回调函数";
        }
        JSValue callret = JS_Call(ctx, jsv, JS_NULL, 0, nullptr);   // 首先调用一个空参获取一个值
        if(JS_IsException(callret)) {
            JS_FreeValue(ctx, callret);
            return "JS函数调用抛出异常";
        }
        auto retv = toFloat(callret);
        JS_FreeValue(ctx, callret);
        if(!retv) {
            return "Call的返回值转为float时失败";
        }
        v = *retv;
        call(&v);

        JSValue canshuv = JS_NewFloat64(ctx, v);
        JSValue canshu[] = { canshuv };
        JS_FreeValue(ctx, JS_Call(ctx, jsv, JS_NULL, 1, canshu));
        JS_FreeValue(ctx, canshuv);
    }
    catch(std::exception& ex) {
        return ex.what();
    }
    return std::string();
}

std::string JSTool::ReferenceDoubleCall(JSValue jsv, std::function<void(double*)> call) {
    try {
        auto ctx = JSManager::getInstance()->getctx();
        double v = 0;
        if(!JS_IsFunction(ctx, jsv)) {
            return "传入的参数不是一个回调函数";
        }
        JSValue callret = JS_Call(ctx, jsv, JS_NULL, 0, nullptr);   // 首先调用一个空参获取一个值
        if(JS_IsException(callret)) {
            JS_FreeValue(ctx, callret);
            return "JS函数调用抛出异常";
        }
        auto retv = toDouble(callret);
        JS_FreeValue(ctx, callret);
        if(!retv) {
            return "Call的返回值转为double时失败";
        }
        v = *retv;
        call(&v);

        JSValue canshuv = JS_NewFloat64(ctx, v);
        JSValue canshu[] = { canshuv };
        JS_FreeValue(ctx, JS_Call(ctx, jsv, JS_NULL, 1, canshu));
        JS_FreeValue(ctx, canshuv);
    }
    catch(std::exception& ex) {
        return ex.what();
    }
    return std::string();
}

// (_ = str:string) => str = _;
std::string JSTool::ReferenceStringCall(JSValue jsv,size_t size, std::function<void(char*)> call) {
    try {
        auto ctx = JSManager::getInstance()->getctx();
        //std::string v = {};
        if(!JS_IsFunction(ctx, jsv)) {
            return "传入的参数不是一个回调函数";
        }
        JSValue callret = JS_Call(ctx, jsv, JS_NULL, 0, nullptr);   // 首先调用一个空参获取一个值
        if(JS_IsException(callret)) {
            JS_FreeValue(ctx, callret);
            return "JS函数调用抛出异常";
        }
        auto retv = toString(callret);
        JS_FreeValue(ctx, callret);
        if(!retv) {
            return "Call的返回值转为string时失败";
        }
        char* text = new char[size];
        if(text == NULL) throw std::runtime_error(std::format("申请{}长度的char数组失败 in {}:{}", size, __FILE__, __LINE__));
        memset(text, '\0', size);
        memcpy_s(text, size, retv->c_str(), retv->size());

        call(text);

        JSValue canshuv = fromString(std::string(text)); delete[] text;
        JSValue canshu[] = { canshuv };
        JS_FreeValue(ctx, JS_Call(ctx, jsv, JS_NULL, 1, canshu));
        JS_FreeValue(ctx, canshuv);
    }
    catch(std::exception& ex) {
        return ex.what();
    }
    return std::string();
}

std::string JSTool::ReferenceVec2PropCall(JSValue jsv, std::function<void(float*)> call) {
    try {
        auto ctx = JSManager::getInstance()->getctx();
        float v2[2] = {0};
        auto getv = getPropXY(jsv);
        if(!getv) throw std::runtime_error("获取对象属性x,y失败");
        v2[0] = getv->at(0);
        v2[1] = getv->at(1);

        call(v2);

        if(!setPropXY(jsv, { v2[0], v2[1] })) {
            throw std::runtime_error("设置对象属性x,y失败");
        }
    }
    catch(std::exception& ex) {
        return ex.what();
    }
    return std::string();
}

std::string JSTool::ReferenceVec3PropCall(JSValue jsv, std::function<void(float*)> call) {
    try {
        auto ctx = JSManager::getInstance()->getctx();
        bool isxyz = true;
        float v3[3] = { 0 };
        auto getv = getPropXYZ(jsv);
        if(!getv) {
            getv = getPropRGB(jsv);
            isxyz = false;
            if(!getv) throw std::runtime_error("获取对象属性x,y,z 或 r,g,b失败");
        }
        v3[0] = getv->at(0);
        v3[1] = getv->at(1);
        v3[2] = getv->at(2);

        call(v3);
        if(isxyz) {
            if(!setPropXYZ(jsv, { v3[0], v3[1], v3[2] })) {
                throw std::runtime_error("设置对象属性x,y,z失败");
            }
        }
        else {
            if(!setPropRGB(jsv, { v3[0], v3[1], v3[2] })) {
                throw std::runtime_error("设置对象属性r,g,b失败");
            }
        }

    }
    catch(std::exception& ex) {
        return ex.what();
    }
    return std::string();
}

std::string JSTool::ReferenceVec4PropCall(JSValue jsv, std::function<void(float*)> call) {
    try {
        auto ctx = JSManager::getInstance()->getctx();
        bool isxyzw = true;
        float v4[4] = { 0 };
        auto getv = getPropXYZW(jsv);
        if(!getv) {
            getv = getPropRGBA(jsv);
            isxyzw = false;
            if(!getv) throw std::runtime_error("获取对象属性x,y,z,w 或 r,g,b,a失败");
        }
        v4[0] = getv->at(0);
        v4[1] = getv->at(1);
        v4[2] = getv->at(2);
        v4[3] = getv->at(3);

        call(v4);

        if(isxyzw) {
            if(!setPropXYZW(jsv, { v4[0], v4[1], v4[2], v4[3] })) {
                throw std::runtime_error("设置对象属性x,y,z,w失败");
            }
        }
        else{
            if(!setPropRGBA(jsv, { v4[0], v4[1], v4[2], v4[3] })) {
                throw std::runtime_error("设置对象属性r,g,b,a失败");
            }
        }

    }
    catch(std::exception& ex) {
        return ex.what();
    }
    return std::string();
}

int JSTool::iMin(size_t v1, int v2) {
    if(v1 > v2) return v2;
    else return static_cast<int>(v1);
}


JSTool::Param JSTool::createParseParameter(int argc, JSValue* argv) {
    return JSTool::Param(argc, argv);
}

JSTool::Param::Param(int argc, JSValue* argv) : m_argc(argc), m_argv(argv) {}

JSTool::Param& JSTool::Param::Parse(bool need) {
    if(m_index >= m_argc && need) { // 不够
        m_hasErr = true;
        m_JSErr = std::format("仅有{}个参数是不够的，正在尝试读取第{}个参数", m_argc, m_index + 1);
    }
    m_index++;
    return *this;
}

JSTool::Param& JSTool::Param::ParseCall(bool need) {
    if(m_index >= m_argc && need) { // 不够
        m_hasErr = true;
        m_JSErr = std::format("仅有{}个参数是不够的，正在尝试读取第{}个参数", m_argc, m_index + 1);
    }
    else if(m_index < m_argc) {
        if(!JS_IsFunction(JSManager::getInstance()->getctx(), m_argv[m_index])) {
            m_hasErr = true;
            m_JSErr = std::format("第{}个参数不是回调函数,错误",m_index + 1);
        }
    }
    m_index++;
    return *this;
}

const std::string JSTool::Param::Build() const {
    return m_JSErr;
}
