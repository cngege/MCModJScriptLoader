#include "JsModule_eventSystem.h"
#include "../JSManager.h"
#include "../client/ModManager.h"
#include <unordered_map>
#include <shared_mutex>
#include <list>
#include <spdlog/spdlog.h>



// 首先有个map 存储监听的事件
static std::unordered_map<std::string, std::list<JSValue>> jsEvent = {};
static std::shared_mutex rw_mtx_eventList;

//std::shared_lock<std::shared_mutex> lock(rw_mtx_clientList); // 共享锁 读锁
//std::unique_lock<std::shared_mutex> lock(rw_mtx_clientList); // 唯一锁 写锁

static JSValue js_listenEvent(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    // 第一个参数-字符串： 事件名称
    // 第二个参数 JS函数

    if(argc >= 2) {
        if(!JS_IsString(argv[0])) {
            return JS_ThrowTypeError(ctx, "函数的参数一必须是字符串");
        }
        auto str = JS_ToCString(ctx, JS_ToString(ctx, argv[0]));
        auto eventname = std::string(str);
        JS_FreeCString(ctx, str);

        if(!JS_IsFunction(ctx, argv[1])) {
            return JS_ThrowTypeError(ctx, "函数的参数二必须是一个函数call");
        }
        // map中查找
        std::unique_lock<std::shared_mutex> lock(rw_mtx_eventList);
        auto it = jsEvent.find(eventname);
        if(it != jsEvent.end()) {
            //有
            it->second.push_back(JS_DupValue(ctx, argv[1]));
        }
        else {
            jsEvent[eventname] = { JS_DupValue(ctx, argv[1]) };
        }
        return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "函数最少需要2个参数，当前参数个数：%d", argc);
}

static JSValue js_removeEvent(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    // 第一个参数-字符串： 事件名称
    // 第二个参数 JS函数

    if(argc >= 2) {
        if(!JS_IsString(argv[0])) {
            return JS_ThrowTypeError(ctx, "函数的参数一必须是字符串");
        }
        auto str = JS_ToCString(ctx, JS_ToString(ctx, argv[0]));
        auto eventname = std::string(str);
        JS_FreeCString(ctx, str);

        if(!JS_IsFunction(ctx, argv[1])) {
            return JS_ThrowTypeError(ctx, "函数的参数二必须是一个函数call");
        }
        // map中查找
        std::unique_lock<std::shared_mutex> lock(rw_mtx_eventList);
        auto it = jsEvent.find(eventname);
        if(it != jsEvent.end()) {
            //有 - 找到并移除
            auto& e = it->second;
            for(auto jsit = e.begin(); jsit != e.end(); ++jsit) {
                if(JS_VALUE_GET_PTR(*jsit) == JS_VALUE_GET_PTR(argv[1])) {
                    JS_FreeValue(ctx, *jsit);
                    e.erase(jsit);
                }
            }
        }
        return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "函数最少需要2个参数，当前参数个数：%d", argc);
}

static JSValue js_broadcastEvent(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    // 第一个参数-字符串： 事件名称
    // 第二个参数-JS回调： 返回值bool, 第二个参数可以是NULL
    // 第三个开始 JS函数的调用参数

    if(argc >= 2) {
        if(!JS_IsString(argv[0])) {
            return JS_ThrowTypeError(ctx, "函数的参数一必须是字符串");
        }
        auto str = JS_ToCString(ctx, JS_ToString(ctx, argv[0]));
        auto eventname = std::string(str);
        JS_FreeCString(ctx, str);

        bool callisNull = JS_IsNull(argv[1]);

        // map中查找
        std::shared_lock<std::shared_mutex> lock(rw_mtx_eventList);
        auto it = jsEvent.find(eventname);
        if(it != jsEvent.end()) {
            //有 - 找到并调用
            auto& e = it->second;
            for(auto& fun : e) {
                auto ret = JS_Call(ctx, fun, this_val, argc - 2, (argc - 2)? argv + 2 : nullptr);
                if(JS_IsException(ret)) {
                    // 如果执行的时候出现了异常
                    // 则停掉这个call的执行
                    lock.unlock();
                    std::unique_lock<std::shared_mutex> lock2(rw_mtx_eventList);

                    spdlog::error(JSManager::getInstance()->getErrorStack().c_str());
                    e.remove_if([=](JSValue& v) { return JS_VALUE_GET_PTR(fun) == JS_VALUE_GET_PTR(v); });
                    spdlog::error("该监听将被移除");
                    return ret;
                }
                JSValue p[] = { ret };

                if(!JS_IsUndefined(ret) && !callisNull && JS_IsFunction(ctx, argv[1])) {
                    JS_BOOL b = JS_ToBool(ctx, ret);
                    JS_Call(ctx, argv[1], this_val, 1, p);
                }
                JS_FreeValue(ctx, ret);
            }
        }
        return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "函数最少需要2个参数，当前参数个数：%d", argc);
}


static const JSCFunctionListEntry js_eventSys_funcs[] = {
    //JS_CFUNC_DEF("Get", 1, js_http_get_request),
        JS_CFUNC_DEF2("监听事件", 1, js_listenEvent),
        JS_CFUNC_DEF2("移除事件", 1, js_removeEvent),
        JS_CFUNC_DEF2("事件广播", 1, js_broadcastEvent),
};

static int js_eventSys_init(JSContext* ctx, JSModuleDef* m) {
    return JS_SetModuleExportList(ctx, m, js_eventSys_funcs, _countof(js_eventSys_funcs));
}

JSModuleDef* js_init_module_eventSystem(JSContext* ctx, const char* module_name) {
    JSModuleDef* m = JS_NewCModule(ctx, module_name, js_eventSys_init);
    if(!m)
        return NULL;
    JS_AddModuleExportList(ctx, m, js_eventSys_funcs, _countof(js_eventSys_funcs));
    return m;
}


// 二进制程序内部广播事件 - 事件没有返回值则不调用回调
void NativeBroadcastEvent(const std::string& name, JSValueConst this_val, int argc, JSValueConst* argv, std::function<void(bool)> callback) {
    std::shared_lock<std::shared_mutex> lock(rw_mtx_eventList);
    auto it = jsEvent.find(name);
    if(it != jsEvent.end()) {
        //有 - 找到并调用
        auto& e = it->second;
        auto ctx = JSManager::getInstance()->getctx();
        for(auto& fun : e) {
            auto ret = JS_Call(ctx, fun, this_val, argc, argv);
            if(JS_IsException(ret)) {
                //ModManager::getInstance()->trySafeExceptions(std::exception(JSManager::getInstance()->getErrorStack().c_str()));
                //JS_FreeValue(JSManager::getInstance()->getctx(), ret);
                lock.unlock();
                std::unique_lock<std::shared_mutex> lock2(rw_mtx_eventList);

                spdlog::error(JSManager::getInstance()->getErrorStack().c_str());
                spdlog::error("该监听将被移除");
                e.remove_if([=](JSValue& v) { return JS_VALUE_GET_PTR(fun) == JS_VALUE_GET_PTR(v); });
                JS_FreeValue(ctx, fun);
                return;
            }

            if(!JS_IsUndefined(ret) && callback != nullptr) {
                JS_BOOL b = JS_ToBool(JSManager::getInstance()->getctx(), ret);
                callback(b);
            }
            JS_FreeValue(ctx, ret);
        }
    }
}
// 二进制程序内部监听事件
JSValue NativeListenEvent(const std::string& eventname, JSCFunction jsfun, const std::string& funname) {
    auto jfun = JS_NewCFunction(JSManager::getInstance()->getctx(), jsfun, funname.c_str(), 0);

    // map中查找
    std::unique_lock<std::shared_mutex> lock(rw_mtx_eventList);
    auto it = jsEvent.find(eventname);
    if(it != jsEvent.end()) {
        //有
        it->second.push_back(jfun);
    }
    else {
        jsEvent[eventname] = { jfun };
    }
    return jfun;	// 用于移除
}

void NativeRemoveEvent(const std::string& eventname, JSValue jsfun) {
    auto ctx = JSManager::getInstance()->getctx();
    JSValue argvs[] = { JS_NewString(ctx, eventname.c_str()), jsfun };
    js_removeEvent(ctx, JS_UNDEFINED, 2, argvs);
}