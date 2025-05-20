#include "JSManager.h"
#include <fstream>
#include <iostream>
#include "../client/ModManager.h"
#include "spdlog/spdlog.h"
#include "eventSystem/JsModule_eventSystem.h"
#include <unordered_map>
#include <shared_mutex>
#include "imgui/appConsole.h"
#include "imgui/imgui.h"
#include "imgui/imgui_uwp_wndProc.h"

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

JSModuleDef* js_init_module_mem(JSContext* ctx, const char* module_name);
JSModuleDef* js_init_module_eventSystem(JSContext* ctx, const char* module_name);
JSModuleDef* js_init_module_imgui(JSContext* ctx, const char* module_name);
JSModuleDef* js_init_module_signcode(JSContext* ctx, const char* module_name);
JSModuleDef* js_init_module_http(JSContext* ctx, const char* module_name);


auto JSManager::loadNativeModule() -> void {
    js_init_module_std(m_ctx, "std");
    js_init_module_os(m_ctx, "os");

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

        JSValue val = JS_Eval(m_ctx, content.c_str(), content.size(), path.c_str(), JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY/* | JS_EVAL_TYPE_MASK*/);
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

// BaseFolder script, load All File
auto JSManager::loadJSFromFoder(const std::string& folder) -> void {
    for(const auto& entry : fs::directory_iterator(ModManager::getInstance()->getPath("script") / folder)) {
        if(entry.is_regular_file() && entry.path().extension() == ".js") {
            std::ifstream jsfile(entry.path());
            std::string content((std::istreambuf_iterator<char>(jsfile)), std::istreambuf_iterator<char>());
            spdlog::info("JS Loader: {}", entry.path().filename().string());
            std::string scriptname = entry.path().filename().string();
            // 新建一个JS对象
            auto gobj = JS_GetGlobalObject(m_ctx);
            onJsLoadBefore(scriptname, gobj);

            JSValue val = JS_EvalThis(m_ctx, gobj, content.c_str(), content.size(), (fs::path(folder) / entry.path().filename()).string().c_str(), JS_EVAL_TYPE_MODULE);
            onJsLoadAfter(scriptname, gobj);
            if(JS_IsException(val)) {
                spdlog::error("JS Loader fail: {}", entry.path().string());
                auto result_str = JS_ErrorStackCheck(m_ctx);
                spdlog::error(result_str);
            }

            JS_FreeValue(m_ctx, val);
        }
    }
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

auto JSManager::runstdLoop() -> void {
    js_std_loop(m_ctx);
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
    return getErrorStack(JS_GetException(m_ctx));
}


auto JSManager::onImGuiRenderScriptSig() -> void {
    if(ImGui::Button("保存")) {
        // TODO:
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

auto JSManager::registerImGuiMouseHandle() -> void {
    registerCoreWindowEventHandle();
}

auto JSManager::initJSManager()->void {
    
}

//void unregisterCoreWindowEventHandle();
auto JSManager::disableJSManager() -> void {
    unregisterCoreWindowEventHandle();
}



//////////////////////// JSTool ////////////////////////

std::optional<std::string> JSTool::toString(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<std::string> str;
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

std::optional<std::vector<JSValue>> JSTool::toArray(JSValue jsv) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<std::vector<JSValue>> ret{};
    if(!JS_IsArray(ctx, jsv)) {
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