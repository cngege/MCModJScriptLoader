#include <string>

#include "../JSManager.h"
#include "../client/utils/signcode.h"
#include "../spdlog/spdlog.h"

namespace {
    static JSClassID id;
    static JSClassDef _SignCodeClass = {
        .class_name{"SignCode"},
        .finalizer{[](JSRuntime* rt, JSValue val) {
                auto signcode = (SignCode*)JS_GetOpaque(val, id);
                delete signcode;
            }
        }
    };
}

static JSValue constructor(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    // 构造函数可以有1-3个参数 [string, bool，bool]
    std::optional<std::string> _title;
    std::optional<bool> _printfail = true;
    std::optional<bool> _checkAllSig = false;
    std::optional<bool> _judgeDifference = true;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(_title)
        .Parse(_printfail)
        .Parse(_checkAllSig)
        .Parse(_judgeDifference)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    SignCode* self = new SignCode(*_title, *_printfail,*_checkAllSig,*_judgeDifference);

    JSValue obj = JSTool::getConstructorValue(newTarget, id);
    JS_SetOpaque(obj, self);
    return obj;

    //if(argc < 1) {
    //    return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    //}
    //auto str = JS_ToCString(ctx, JS_ToString(ctx, argv[1]));
    //std::string title = std::string(str);
    //JS_FreeCString(ctx, str);

    //bool printfail = true;
    //if(argc >= 2) {
    //    if(!JS_IsBool(argv[1])) {
    //        return JS_ThrowTypeError(ctx, "第二个参数必须为bool类型");
    //    }
    //    printfail = JS_ToBool(ctx, argv[1]);
    //}

    //SignCode* self = new SignCode(title.c_str(), printfail);

    //JSValue obj = JS_NewObjectClass(ctx, id);
    //JS_SetOpaque(obj, self);
    //return obj;
}

static JSValue isOK(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    SignCode* thi = (SignCode*)JS_GetOpaque(newTarget, id);
    if(*thi) {
        return JS_TRUE;
    }
    else {
        return JS_FALSE;
    }
}

static JSValue get(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    SignCode* thi = (SignCode*)JS_GetOpaque(newTarget, id);
    return JS_NewInt64(ctx, thi->get());
}

static JSValue ValidSign(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    SignCode* thi = (SignCode*)JS_GetOpaque(newTarget, id);
    JSValue jsv = JSTool::fromString(thi->ValidSign());
    return jsv;
}

static JSValue ValidPtr(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    SignCode* thi = (SignCode*)JS_GetOpaque(newTarget, id);
    return JS_NewInt64(ctx, thi->ValidPtr());
}

static JSValue setDeepSearch(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    SignCode* thi = (SignCode*)JS_GetOpaque(newTarget, id);
    if(argc >= 1) {
        if(!JS_IsBool(argv[0])) return JS_ThrowTypeError(ctx, "参数1仅接受Bool值");
        auto b = JS_ToBool(ctx, argv[0]);
        thi->setDeepSearch(b);
    }
    else {
        thi->setDeepSearch();
    }
    return JS_UNDEFINED;
}

static JSValue AddSign(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    SignCode* thi = (SignCode*)JS_GetOpaque(newTarget, id);
    if(argc >= 1) {
        auto str = JSTool::toString(argv[0]);
        if(!str) {
            return JS_ThrowTypeError(ctx, "首个参数应该为字符串");
        }

        if(argc >= 2) {
            if(!JS_IsFunction(ctx, argv[1])) {
                return JS_ThrowTypeError(ctx, "第二个参数应该为回调函数(number)=>number");
            }
            thi->AddSign(str.value().c_str(), [&](uintptr_t v) {
                JSValue jsv[] = { JS_NewInt64(ctx, v) };
                JSValue callret = JS_Call(ctx, argv[1], newTarget, 1, jsv);
                auto ret = JSTool::toInt64(callret);
                if(!ret) {
                    spdlog::error("回调函数不是number类型(int64)");
                    return v;
                }
                return (uintptr_t)ret.value();
            });
        }
        else {
            thi->AddSign(str.value().c_str());
        }
        return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "函数至少需要1个参数，当前参数个数：%d", argc);
}

static JSValue AddSignCall(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    SignCode* thi = (SignCode*)JS_GetOpaque(newTarget, id);
    if(argc >= 1) {
        auto str = JSTool::toString(argv[0]);
        if(!str) {
            return JS_ThrowTypeError(ctx, "首个参数应该为字符串");
        }
        int offset = 1;
        if(argc >= 2) {
            auto _offset = JSTool::toInt(argv[1]);
            if(!offset) {
                return JS_ThrowTypeError(ctx, "第二个参数必须为int类型");
            }
            offset = *_offset;
        }

        if(argc >= 3) {
            if(!JS_IsFunction(ctx, argv[2])) {
                return JS_ThrowTypeError(ctx, "第三个参数应该为回调函数(number)=>number");
            }
            thi->AddSignCall(str.value().c_str(), offset, [&](uintptr_t v) {
                JSValue jsv[] = { JS_NewInt64(ctx, v) };
                JSValue callret = JS_Call(ctx, argv[2], newTarget, 1, jsv);
                auto ret = JSTool::toInt64(callret);
                if(!ret) {
                    spdlog::error("回调函数返回值不是number类型(int64)");
                    return v;
                }
                return (uintptr_t)ret.value();
            });
        }
        else {
            thi->AddSignCall(str.value().c_str(), offset);
        }
        return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "函数至少需要1个参数，当前参数个数：%d", argc);
}


static auto SignCode_Module_Reg() -> JSValue {
    return JSTool::JSClassRegister(&id, &_SignCodeClass, _SignCodeClass.class_name)
        .setPropFunc(isOK, "isOK")
        .setPropFunc(get, "get")
        .setPropFunc(ValidSign, "ValidSign")
        .setPropFunc(ValidPtr, "ValidPtr")
        .setPropFunc(setDeepSearch, "setDeepSearch")
        .setPropFunc(AddSign, "AddSign")
        .setPropFunc(AddSignCall, "AddSignCall")
        .setConstructor(constructor)
        .buildToModule();

    //JSContext* ctx = JSManager::getInstance()->getctx();
    //auto rt = JS_GetRuntime(ctx);
    //JS_NewClassID(&id);
    //JS_NewClass(rt, id, &_SignCodeClass);

    //JSValue protoInstance = JS_NewObject(ctx);
    //JS_SetPropertyStr(ctx, protoInstance, "isOK", JS_NewCFunction(ctx, isOK, "isOK", 0));
    //JS_SetPropertyStr(ctx, protoInstance, "get", JS_NewCFunction(ctx, get, "get", 0));
    //JS_SetPropertyStr(ctx, protoInstance, "ValidSign", JS_NewCFunction(ctx, ValidSign, "ValidSign", 0));
    //JS_SetPropertyStr(ctx, protoInstance, "ValidPtr", JS_NewCFunction(ctx, ValidPtr, "ValidPtr", 0));
    //JS_SetPropertyStr(ctx, protoInstance, "AddSign", JS_NewCFunction(ctx, AddSign, "AddSign", 0));
    //JS_SetPropertyStr(ctx, protoInstance, "AddSignCall", JS_NewCFunction(ctx, AddSignCall, "AddSignCall", 0));

    //JSValue ctroInstance = JS_NewCFunction2(ctx, &constructor, _SignCodeClass.class_name, 0, JS_CFUNC_constructor, 0);
    //JS_SetConstructor(ctx, ctroInstance, protoInstance);
    //JS_SetClassProto(ctx, id, protoInstance);

    //return ctroInstance;
}

static int js_signcode_init(JSContext* ctx, JSModuleDef* m) {
    static auto jv = SignCode_Module_Reg();
    return JS_SetModuleExport(ctx, m, "default", jv);
}

JSModuleDef* js_init_module_signcode(JSContext* ctx, const char* module_name) {
    JSModuleDef* m = JS_NewCModule(ctx, module_name, js_signcode_init);
    if(!m)
        return NULL;
    JS_AddModuleExport(ctx, m, "default");
    return m;
}