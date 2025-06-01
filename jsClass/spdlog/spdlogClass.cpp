#include "spdlogClass.h"
#include "spdlog/spdlog.h"
#include "../JSManager.h"


namespace {
    static JSClassID id;
    static JSClassDef _spdlogClass = {
        .class_name{"spdlog"},
        .finalizer{[](JSRuntime* rt, JSValue val) {
                auto class_ptr = (std::string*)JS_GetOpaque(val, id);
                if(class_ptr != nullptr) {
                    delete class_ptr;
                }
            }
        }
    };
}

static std::string readStr(JSContext* ctx, int argc, JSValue* argv) {
    std::string allstr;
    for(int i = 0; i < argc; i++) {
        auto str = JS_ToCString(ctx, JS_ToString(ctx, argv[i]));
        if(i) {
            allstr += " ";
        }
        allstr += str;
        JS_FreeCString(ctx, str);
    }
    return allstr;
}

void spdlogClass::Reg() {
    JSTool::JSClassRegister(&id, &_spdlogClass, _spdlogClass.class_name)
        .setPropFunc(spdlogClass::info, "info")
        .setPropFunc(spdlogClass::warn, "warn")
        .setPropFunc(spdlogClass::error, "error")
        .setPropFunc(spdlogClass::debug, "debug")
        .setPropFunc(spdlogClass::trace, "trace")
        .setConstructor(&spdlogClass::constructor)
        .build();

    //JSContext* ctx = JSManager::getInstance()->getctx();
    //auto rt = JS_GetRuntime(ctx);
    //JS_NewClassID(&id);
    //JS_NewClass(rt, id, &_spdlogClass);

    //JSValue protoInstance = JS_NewObject(ctx);
    //JS_SetPropertyStr(ctx, protoInstance, "info", JS_NewCFunction(ctx, spdlogClass::info, "info", 1));
    //JS_SetPropertyStr(ctx, protoInstance, "warn", JS_NewCFunction(ctx, spdlogClass::warn, "warn", 1));
    //JS_SetPropertyStr(ctx, protoInstance, "error", JS_NewCFunction(ctx, spdlogClass::error, "error", 1));
    //JS_SetPropertyStr(ctx, protoInstance, "debug", JS_NewCFunction(ctx, spdlogClass::debug, "debug", 1));
    //JS_SetPropertyStr(ctx, protoInstance, "trace", JS_NewCFunction(ctx, spdlogClass::trace, "trace", 1));

    //JSValue ctroInstance = JS_NewCFunction2(ctx, &spdlogClass::constructor, _spdlogClass.class_name, 0, JS_CFUNC_constructor, 0);
    //JS_SetConstructor(ctx, ctroInstance, protoInstance);
    //JS_SetClassProto(ctx, id, protoInstance);

    //JSValue global_obj = JS_GetGlobalObject(ctx);
    //JS_SetPropertyStr(ctx, global_obj, _spdlogClass.class_name, ctroInstance);
    //JS_FreeValue(ctx, global_obj);
    //JS_FreeValue(ctx, ctroInstance);
}

void spdlogClass::Dispose() {
    
}

JSValue spdlogClass::constructor(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    JSValue obj = JSTool::getConstructorValue(newTarget, id);
    if(argc >= 1) {
        auto hasStr = JSTool::toString(argv[0]);
        if(!hasStr) {
            return JS_ThrowTypeError(ctx, "首个参数仅可以使用能转为字符串的值");
        }
        std::string* str = new std::string(hasStr.value());
        JS_SetOpaque(obj, str);
    }
    else {
        JS_SetOpaque(obj, nullptr);
    }
    return obj;
}

JSValue spdlogClass::info(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
    auto allstr = readStr(ctx, argc, argv);
    auto class_ptr = (std::string*)JS_GetOpaque(thisVal, id);
    if(class_ptr == nullptr) {
        spdlog::info("{}", allstr);
    }
    else {
        spdlog::info("[{}] {}",class_ptr->c_str(), allstr);
    }
    return JS_UNDEFINED;
}

JSValue spdlogClass::warn(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
    auto allstr = readStr(ctx, argc, argv);
    auto class_ptr = (std::string*)JS_GetOpaque(thisVal, id);
    if(class_ptr == nullptr) {
        spdlog::warn(allstr);
    }
    else {
        spdlog::warn("[{}] {}", class_ptr->c_str(), allstr);
    }
    return JS_UNDEFINED;
}

JSValue spdlogClass::error(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
    auto allstr = readStr(ctx, argc, argv);
    auto class_ptr = (std::string*)JS_GetOpaque(thisVal, id);
    if(class_ptr == nullptr) {
        spdlog::error(allstr);
    }
    else {
        spdlog::error("[{}] {}", class_ptr->c_str(), allstr);
    }
    return JS_UNDEFINED;
}

JSValue spdlogClass::debug(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
    auto allstr = readStr(ctx, argc, argv);
    auto class_ptr = (std::string*)JS_GetOpaque(thisVal, id);
    if(class_ptr == nullptr) {
        spdlog::debug(allstr);
    }
    else {
        spdlog::debug("[{}] {}", class_ptr->c_str(), allstr);
    }
    return JS_UNDEFINED;
}

JSValue spdlogClass::trace(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
    auto allstr = readStr(ctx, argc, argv);
    auto class_ptr = (std::string*)JS_GetOpaque(thisVal, id);
    if(class_ptr == nullptr) {
        spdlog::trace(allstr);
    }
    else {
        spdlog::trace("[{}] {}", class_ptr->c_str(), allstr);
    }
    return JS_UNDEFINED;
}