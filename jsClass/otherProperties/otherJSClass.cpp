
#include "otherJSClass.h"
#include "../JSManager.h"
#include "../hook/hookClass.h"
#include <thread>
//#include <chrono>

JSValue createThread(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc >= 1) {
        if(!JS_IsFunction(ctx, argv[0])) {
            return JS_ThrowTypeError(ctx, "参数1应传递函数");
        }
        JSValue fun = JS_DupValue(ctx, argv[0]);
        JSValue thisobj = JS_DupValue(ctx, newTarget);

        INT32 sleepTime = 0;
        if(argc >= 2) {
            if(!JS_IsNumber(argv[1]) || JS_ToInt32(ctx, &sleepTime, argv[1]) < 0) {
                return JS_ThrowTypeError(ctx, "参数2请传入一个合法的Number");
            }
        }
        std::thread t([sleepTime, ctx, fun, thisobj]() {
            if(sleepTime > 0) std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            JS_Call(ctx, fun, thisobj, 0, nullptr);
            JS_FreeValue(ctx, thisobj);
            JS_FreeValue(ctx, fun);
        });
        t.detach();
        return JS_NewInt32(ctx, 0);
    }
    return JS_ThrowTypeError(ctx, "至少需要2个参数,类型为[Function, Number]");
}

JSValue thread_sleep(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc >= 1) {
        INT32 sleepTime = 0;
        if(!JS_IsNumber(argv[0]) || JS_ToInt32(ctx, &sleepTime, argv[0]) < 0) {
            return JS_ThrowTypeError(ctx, "参数1请传入一个合法的Number");
        }
        if(sleepTime > 0) std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "至少需要1个参数,类型为[Number]");
}

JSValue testDumpValue(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc >= 1) {
        return JS_DupValue(ctx, argv[0]);
    }
    return JS_ThrowTypeError(ctx, "至少需要1个参数");
}


JSValue setTimeout(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    std::optional<int64_t> t;
    auto ret = JSTool::createParseParameter(argc, argv)
        .ParseCall()
        .Parse(t)
        .Build();
    if(!ret.empty()) {
        return JS_ThrowTypeError(ctx, ret.c_str());
    }

    return JS_NewInt64(ctx, JSManager::getInstance()->addTimeOut(argv[0], *t));
}

JSValue setInterval(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    std::optional<int64_t> t;
    auto ret = JSTool::createParseParameter(argc, argv)
        .ParseCall()
        .Parse(t)
        .Build();
    if(!ret.empty()) {
        return JS_ThrowTypeError(ctx, ret.c_str());
    }

    return JS_NewInt64(ctx, JSManager::getInstance()->addInterval(argv[0], *t));
}

JSValue clearTimeout(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    std::optional<int64_t> id;
    auto ret = JSTool::createParseParameter(argc, argv)
        .Parse(id)
        .Build();
    if(!ret.empty()) {
        return JS_ThrowTypeError(ctx, ret.c_str());
    }
    JSManager::getInstance()->removeTask(*id);
    return JS_UNDEFINED;
}

void otherJSClass::Reg() {
    auto ctx = JSManager::getInstance()->getctx();
    JSValue obj = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, obj, "createThread", JS_NewCFunction(ctx, createThread, "createThread", 2));
    JS_SetPropertyStr(ctx, obj, "threadSleep", JS_NewCFunction(ctx, thread_sleep, "threadSleep", 1));
    JS_SetPropertyStr(ctx, obj, "testDumpValue", JS_NewCFunction(ctx, testDumpValue, "testDumpValue", 1));

    JS_SetPropertyStr(ctx, obj, "setTimeout", JS_NewCFunction(ctx, setTimeout, "setTimeout", 2));
    JS_SetPropertyStr(ctx, obj, "setInterval", JS_NewCFunction(ctx, setInterval, "setInterval", 2));
    JS_SetPropertyStr(ctx, obj, "clearTimeout", JS_NewCFunction(ctx, clearTimeout, "clearTimeout", 1));
    JS_SetPropertyStr(ctx, obj, "clearInterval", JS_NewCFunction(ctx, clearTimeout, "clearInterval", 1));
    JS_FreeValue(ctx, obj);
}

void otherJSClass::Dispose() {}


