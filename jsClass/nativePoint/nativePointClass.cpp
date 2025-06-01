#include "nativePointClass.h"
#include "dyncall/dyncall_callback.h"
#include "spdlog/spdlog.h"
#include "../JSManager.h"
#include "../client/mem/mem.h"

#define NOMINMAX


namespace {
    //static JSClassID id;
    static JSClassDef _nativePointClass = {
        .class_name{"NativePoint"},
        .finalizer{[](JSRuntime* rt, JSValue val) {
                auto NativePoint = (nativePointClass*)JS_GetOpaque(val, nativePointClass::id);
                delete NativePoint;
            }
        }
    };
}

JSClassID nativePointClass::id{};

void nativePointClass::Reg() {
    auto ctx = JSManager::getInstance()->getctx();
    JSTool::JSClassRegister(&nativePointClass::id, &_nativePointClass, _nativePointClass.class_name)
        .setPropFunc(nativePointClass::call, "call")
        .setPropFunc(nativePointClass::offset, "offset")
        .setPropFunc(nativePointClass::toNumber, "toNumber")
        .setPropFunc(nativePointClass::setAgree, "setAgree")
        .setPropFunc(nativePointClass::setbool, "setbool")
        .setPropFunc(nativePointClass::getbool, "getbool")
        .setPropFunc(nativePointClass::setchar, "setchar")
        .setPropFunc(nativePointClass::getchar, "getchar")
        .setPropFunc(nativePointClass::setuchar, "setuchar")
        .setPropFunc(nativePointClass::getuchar, "getuchar")
        .setPropFunc(nativePointClass::setshort, "setshort")
        .setPropFunc(nativePointClass::getshort, "getshort")
        .setPropFunc(nativePointClass::setushort, "setushort")
        .setPropFunc(nativePointClass::getushort, "getushort")
        .setPropFunc(nativePointClass::setint, "setint")
        .setPropFunc(nativePointClass::getint, "getint")
        .setPropFunc(nativePointClass::setuint, "setuint")
        .setPropFunc(nativePointClass::getuint, "getuint")
        .setPropFunc(nativePointClass::setlong, "setlong")
        .setPropFunc(nativePointClass::getlong, "getlong")
        .setPropFunc(nativePointClass::setulong, "setulong")
        .setPropFunc(nativePointClass::getulong, "getulong")
        .setPropFunc(nativePointClass::setfloat, "setfloat")
        .setPropFunc(nativePointClass::getfloat, "getfloat")
        .setPropFunc(nativePointClass::setdouble, "setdouble")
        .setPropFunc(nativePointClass::getdouble, "getdouble")
        .setPropFunc(nativePointClass::setpoint, "setpoint")
        .setPropFunc(nativePointClass::getpoint, "getpoint")
        .setPropFunc(nativePointClass::getstring, "getstring")
        .setPropFunc(nativePointClass::setstring, "setstring")
        .setPropFunc(nativePointClass::setbytes, "setbytes")
        .setPropFunc(nativePointClass::getCstring, "getcstring")
        .setConstructor(&nativePointClass::constructor)
        .build();

    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue nativeTypes = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, nativeTypes, "Void", JS_NewInt32(ctx, (int)NativeTypes::Void));
    JS_SetPropertyStr(ctx, nativeTypes, "Bool", JS_NewInt32(ctx, (int)NativeTypes::Bool));
    JS_SetPropertyStr(ctx, nativeTypes, "Char", JS_NewInt32(ctx, (int)NativeTypes::Char));
    JS_SetPropertyStr(ctx, nativeTypes, "UnsignedChar", JS_NewInt32(ctx, (int)NativeTypes::UnsignedChar));
    JS_SetPropertyStr(ctx, nativeTypes, "Short", JS_NewInt32(ctx, (int)NativeTypes::Short));
    JS_SetPropertyStr(ctx, nativeTypes, "UnsignedShort", JS_NewInt32(ctx, (int)NativeTypes::UnsignedShort));
    JS_SetPropertyStr(ctx, nativeTypes, "Int", JS_NewInt32(ctx, (int)NativeTypes::Int));
    JS_SetPropertyStr(ctx, nativeTypes, "UnsignedInt", JS_NewInt32(ctx, (int)NativeTypes::UnsignedInt));
    JS_SetPropertyStr(ctx, nativeTypes, "Long", JS_NewInt32(ctx, (int)NativeTypes::Long));
    JS_SetPropertyStr(ctx, nativeTypes, "UnsignedLong", JS_NewInt32(ctx, (int)NativeTypes::UnsignedLong));
    JS_SetPropertyStr(ctx, nativeTypes, "LongLong", JS_NewInt32(ctx, (int)NativeTypes::LongLong));
    JS_SetPropertyStr(ctx, nativeTypes, "UnsignedLongLong", JS_NewInt32(ctx, (int)NativeTypes::UnsignedLongLong));
    JS_SetPropertyStr(ctx, nativeTypes, "Float", JS_NewInt32(ctx, (int)NativeTypes::Float));
    JS_SetPropertyStr(ctx, nativeTypes, "Double", JS_NewInt32(ctx, (int)NativeTypes::Double));
    JS_SetPropertyStr(ctx, nativeTypes, "Pointer", JS_NewInt32(ctx, (int)NativeTypes::Pointer));
    JS_SetPropertyStr(ctx, global_obj, "NativeTypes", nativeTypes);
    // 这个不能释放，释放就崩溃
    //JS_FreeValue(ctx, nativeTypes);
    JS_FreeValue(ctx, global_obj);

    //JS_NewClassID(&nativePointClass::id);
    //JS_NewClass(JS_GetRuntime(ctx), nativePointClass::id, &_nativePointClass);
    //

    //JSValue protoInstance = JS_NewObject(ctx);
    //JSTool::setPropFunc(protoInstance, nativePointClass::call, "call");
    //JSTool::setPropFunc(protoInstance, nativePointClass::offset, "offset");
    //JSTool::setPropFunc(protoInstance, nativePointClass::offset, "add");
    //JSTool::setPropFunc(protoInstance, nativePointClass::toNumber, "toNumber");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setAgree, "setAgree");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setbool, "setbool");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getbool, "getbool");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setchar, "setchar");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getchar, "getchar");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setuchar, "setuchar");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getuchar, "getuchar");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setshort, "setshort");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getshort, "getshort");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setushort, "setushort");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getushort, "getushort");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setint, "setint");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getint, "getint");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setuint, "setuint");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getuint, "getuint");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setlong, "setlong");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getlong, "getlong");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setulong, "setulong");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getulong, "getulong");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setfloat, "setfloat");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getfloat, "getfloat");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setdouble, "setdouble");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getdouble, "getdouble");
    //JSTool::setPropFunc(protoInstance, nativePointClass::setpoint, "setpoint");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getpoint, "getpoint");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getstring, "getstring");
    //JSTool::setPropFunc(protoInstance, nativePointClass::getCstring, "getcstring");

    //// 创建此类的构造函数
    //JSValue ctroInstance = JS_NewCFunction2(ctx, &nativePointClass::constructor, _nativePointClass.class_name, 1, JS_CFUNC_constructor, 0);
    //JS_SetConstructor(ctx, ctroInstance, protoInstance);
    //JS_SetClassProto(ctx, id, protoInstance);

    //// 将此类注册到公共对象上
    //JSValue global_obj2 = JS_GetGlobalObject(ctx);
    //JS_SetPropertyStr(ctx, global_obj2, _nativePointClass.class_name, ctroInstance);

    //JS_FreeValue(ctx, global_obj2);
    //JS_FreeValue(ctx, ctroInstance);
    //JS_FreeValue(ctx, protoInstance);
    
}

void nativePointClass::Dispose() {}

JSValue nativePointClass::FromPtr(uintptr_t ptr) {
    JSValue obj = JS_NewObjectClass(JSManager::getInstance()->getctx(), id);
    JS_SetOpaque(obj, (void*)ptr);
    return obj;
}

JSValue nativePointClass::newNativePoint(uintptr_t ptr) {
    auto thi = new nativePointClass(ptr);
    return FromPtr((uintptr_t)thi);
}

JSValue nativePointClass::newNativePoint(uintptr_t ptr, std::vector<NativeTypes> v) {
    auto thi = new nativePointClass(ptr);
    thi->m_agreeOn = v;
    return FromPtr((uintptr_t)thi);
}

nativePointClass::nativePointClass(uintptr_t ptr) {
    m_ptr = ptr;
    m_vm = dcNewCallVM(1024);
    dcMode(m_vm, DC_CALL_C_DEFAULT);
    dcReset(m_vm);
}

nativePointClass::nativePointClass(uintptr_t ptr, UINT calloc) {
    m_ptr = ptr;
    m_vm = dcNewCallVM(1024);
    m_freelen = calloc;
    dcMode(m_vm, DC_CALL_C_DEFAULT);
    dcReset(m_vm);
}

nativePointClass::~nativePointClass() {
    dcFree(m_vm);
    if(m_ptr && m_freelen > 0) {
        free((void*)m_ptr);
        m_ptr = NULL;
        m_freelen = 0;
    }
}

uintptr_t nativePointClass::get() {
    return m_ptr;
}

JSValue nativePointClass::constructor(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    // 至少一个参数 指针
    // 可选第二个参数 当指针作为call时的调用约定
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数至少需要1个参数，当前参数个数：%d", argc);
    }
    // 处理第一个参数
    int64_t ptr;
    if(!JS_IsNumber(argv[0]) || JS_ToInt64(ctx, &ptr, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为指针，类型应为Number");
    }

    nativePointClass* self = nullptr;
    int64_t memsize = 0;
    if(ptr == 0 && argc >= 2 && JS_IsNumber(argv[1]) && JS_ToInt64(ctx, &memsize, argv[1]) >= 0 && memsize > 0) {
        ptr = (int64_t)calloc(memsize, 1);
        self = new nativePointClass(ptr, static_cast<UINT>(memsize));
        JSValue obj = JSTool::getConstructorValue(newTarget, id);
        JS_SetOpaque(obj, (void*)(uintptr_t)self);
        return obj;
        //return FromPtr((uintptr_t)self);
    }

    self = new nativePointClass(ptr);
    if(argc >= 2) {
        // 处理第二个参数
        try {
            auto arr = JSTool::getArray<NativeTypes>(argv[1], [&](size_t i, JSValue item) {
                int32_t value = 0;
                int r = JS_ToInt32(ctx, &value, item);
                if(r < 0) {
                    throw std::runtime_error(std::format("参数二NativeTypes数组解析失败 i: {}", i));
                }
                return (NativeTypes)value;
            });
            if(!arr) {
                return JS_ThrowTypeError(ctx, "参数二NativeTypes数组解析失败");
            }
            self->m_agreeOn = *arr;
        }
        catch(std::runtime_error& err) {
            return JS_ThrowTypeError(ctx, err.what());
        }
    }
    JSValue obj = JSTool::getConstructorValue(newTarget, id);
    JS_SetOpaque(obj, (void*)(uintptr_t)self);
    return obj;
}

JSValue nativePointClass::offset(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    INT32 offset;
    if(!JS_IsNumber(argv[0]) || JS_ToInt32(ctx, &offset, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为整数，类型应为Number");
    }
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    // 创建一个副本
    return newNativePoint(thi->m_ptr + offset);
}

JSValue nativePointClass::addvtf(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    //todo: 
    return JSValue();
}

JSValue nativePointClass::setAgree(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    // 设置调用约定
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);

    try {
        auto arr = JSTool::getArray<NativeTypes>(argv[0], [&](size_t i, JSValue item) {
            int32_t value = 0;
            int r = JS_ToInt32(ctx, &value, item);
            if(r < 0) {
                throw std::runtime_error(std::format("参数二NativeTypes数组解析失败 i: {}", i));
            }
            return (NativeTypes)value;
        });
        if(!arr) {
            return JS_ThrowTypeError(ctx, "参数一NativeTypes数组解析失败");
        }
        thi->m_agreeOn.clear();
        thi->m_agreeOn = *arr;
    }
    catch(std::runtime_error& err) {
        return JS_ThrowTypeError(ctx, err.what());
    }
    return JS_DupValue(ctx, newTarget);
}

JSValue nativePointClass::setbool(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    if(!JS_IsBool(argv[0])) {
        return JS_ThrowTypeError(ctx, "参数一应为布尔值，类型应为bool");
    }
    int r = JS_ToBool(ctx, argv[0]);
    if(r < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为布尔值，类型应为bool");
    }
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    //*(bool*)thi->m_ptr = (bool)r;
    Mem::setValue<bool>(thi->m_ptr, r);
    return JS_UNDEFINED;
}

JSValue nativePointClass::getbool(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    return JS_NewBool(ctx, Mem::getValue<bool>(thi->m_ptr));
}


JSValue nativePointClass::setchar(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    INT32 value;
    if(!JS_IsNumber(argv[0]) || JS_ToInt32(ctx, &value, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为Char，类型应为Number");
    }
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    Mem::setValue<char>(thi->m_ptr, value);
    return JS_UNDEFINED;
}



JSValue nativePointClass::getchar(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    return JS_NewInt32(ctx, (int32_t)Mem::getValue<char>(thi->m_ptr));
}

JSValue nativePointClass::setuchar(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    uint32_t value;
    if(!JS_IsNumber(argv[0]) || JS_ToUint32(ctx, &value, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为UChar，类型应为Number");
    }
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    Mem::setValue<unsigned char>(thi->m_ptr, value);
    return JS_UNDEFINED;
}



JSValue nativePointClass::getuchar(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    return JS_NewInt32(ctx, (int32_t)Mem::getValue<unsigned char>(thi->m_ptr));
}

JSValue nativePointClass::setshort(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    INT32 value;
    if(!JS_IsNumber(argv[0]) || JS_ToInt32(ctx, &value, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为Short，类型应为Number");
    }
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    Mem::setValue<short>(thi->m_ptr, value);
    return JS_UNDEFINED;
}

JSValue nativePointClass::getshort(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    return JS_NewInt32(ctx, Mem::getValue<short>(thi->m_ptr));
}

JSValue nativePointClass::setushort(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    uint32_t value;
    if(!JS_IsNumber(argv[0]) || JS_ToUint32(ctx, &value, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为UShort，类型应为Number");
    }
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    Mem::setValue<unsigned short>(thi->m_ptr, value);
    return JS_UNDEFINED;
}

JSValue nativePointClass::getushort(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    return JS_NewUint32(ctx, Mem::getValue<unsigned short>(thi->m_ptr));
}

JSValue nativePointClass::setint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    INT32 value;
    if(!JS_IsNumber(argv[0]) || JS_ToInt32(ctx, &value, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为整数，类型应为Number");
    }
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    Mem::setValue<int>(thi->m_ptr, value);
    return JS_UNDEFINED;
}

JSValue nativePointClass::getint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    return JS_NewInt32(ctx, Mem::getValue<int>(thi->m_ptr));
}

JSValue nativePointClass::setuint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    uint32_t value;
    if(!JS_IsNumber(argv[0]) || JS_ToUint32(ctx, &value, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为无符号整数，类型应为Number");
    }
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    Mem::setValue<unsigned int>(thi->m_ptr, value);
    return JS_UNDEFINED;
}

JSValue nativePointClass::getuint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    return JS_NewUint32(ctx, Mem::getValue<unsigned int>(thi->m_ptr));
}

JSValue nativePointClass::setlong(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    INT64 value;
    if(!JS_IsNumber(argv[0]) || JS_ToInt64(ctx, &value, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为长整数，类型应为Number");
    }
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    Mem::setValue<long long>(thi->m_ptr, value);
    return JS_UNDEFINED;
}

JSValue nativePointClass::getlong(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    return JS_NewInt64(ctx, Mem::getValue<long long>(thi->m_ptr));
}

JSValue nativePointClass::setulong(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    INT64 _value;
    if(!JS_IsNumber(argv[0]) || JS_ToInt64(ctx, &_value, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为无符号长整数，类型应为Number");
    }
    uint64_t value = (UINT64)_value;
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    Mem::setValue<unsigned long long>(thi->m_ptr, value);
    return JS_UNDEFINED;
}

// JS实际上不支持无符号的长整型
JSValue nativePointClass::getulong(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    if(Mem::getValue<long long>(thi->m_ptr) < 0) {
        return JS_ThrowTypeError(ctx, "JS无法存储无符号长整型, JS的Number仅能存储双精度浮点数范围的值");
    }
    return JS_NewInt64(ctx, Mem::getValue<unsigned long long>(thi->m_ptr));
}

JSValue nativePointClass::setfloat(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    double value;
    if(JS_ToFloat64(ctx, &value, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为浮点数，类型应为float");
    }
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    Mem::setValue<float>(thi->m_ptr, static_cast<float>(value));
    return JS_UNDEFINED;
}

JSValue nativePointClass::getfloat(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    return JS_NewFloat64(ctx, (double)Mem::getValue<float>(thi->m_ptr));
}

JSValue nativePointClass::setdouble(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    double value;
    if(JS_ToFloat64(ctx, &value, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为浮点数，类型应为double");
    }
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    Mem::setValue<double>(thi->m_ptr, value);
    return JS_UNDEFINED;
}

JSValue nativePointClass::getdouble(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    return JS_NewFloat64(ctx, Mem::getValue<double>(thi->m_ptr));
}


JSValue nativePointClass::setpoint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    
    nativePointClass* thii = (nativePointClass*)JS_GetOpaque(argv[0], id);
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    Mem::setValue<uintptr_t>(thi->m_ptr, thii->m_ptr);
    return JS_UNDEFINED;
}

JSValue nativePointClass::getpoint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    auto ptr = Mem::getValue<uintptr_t>(thi->m_ptr);
    return newNativePoint(ptr);
}

JSValue nativePointClass::getstring(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    std::string& str = *(std::string*)thi->m_ptr;
    return JS_NewString(ctx, str.c_str());
}

JSValue nativePointClass::setstring(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    if(argc < 1) return JS_ThrowTypeError(ctx, "需要接收一个字符串参数");
    auto str = JSTool::toString(argv[0]);
    if(!str)  return JS_ThrowTypeError(ctx, "参数无法转为字符串");
    *(std::string*)thi->m_ptr = *str;
    return JS_UNDEFINED;
}

JSValue nativePointClass::setbytes(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    if(argc < 1) return JS_ThrowTypeError(ctx, "需要接收一个字符串参数");
    try {
        auto str = JSTool::getArray<char8_t>(argv[0], [=](size_t size, JSValue jsv) {
            auto toint = JSTool::toInt(jsv);
            if(!toint) throw std::runtime_error("无法转为Int(Char)类型");
            if(*toint > 255 || *toint < -127) throw std::runtime_error("参数值超出范围, 请限定在char值范围");
            return static_cast<char8_t>(*toint);
        });
        if(!str) {
            throw std::runtime_error("异常情况，无法转为char数组");
        }
        int i = 0;
        for(auto item : *str) {
            *(char*)(thi->m_ptr + i) = item;
            i++;
        }
    }
    catch(std::exception& ex) {
        return JS_ThrowTypeError(ctx, ex.what());
    }
    return JS_UNDEFINED;
}

JSValue nativePointClass::getCstring(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    auto str = (const char*)thi->m_ptr;
    return JS_NewString(ctx, str);
}

////////////////////////////// CALL /////////////////////////////////
JSValue nativePointClass::call(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    JSValue ret = JS_UNDEFINED;
    try {
        auto ori = (void*)thi->m_ptr;
        dcReset(thi->m_vm);
        for(int i = 1; i < thi->m_agreeOn.size(); ++i) {
            switch(thi->m_agreeOn[i]) {
            case NativeTypes::Bool:
            {
                dcArgBool(thi->m_vm, JS_ToBool(ctx, argv[i - 1]));
            }
            break;
            case NativeTypes::Char:
            case NativeTypes::UnsignedChar:
            {
                auto v = JSTool::toInt(argv[i - 1]);
                if(!v) throw std::runtime_error("NativePoint.call(NativeTypes::UnsignedChar(Char))解析失败");
                dcArgChar(thi->m_vm, *v);
            }
            break;
            case NativeTypes::Short:
            case NativeTypes::UnsignedShort:
            {
                auto v = JSTool::toInt(argv[i - 1]);
                if(!v) throw std::runtime_error("NativePoint.call(NativeTypes::UnsignedShort(Short))解析失败");
                dcArgShort(thi->m_vm, *v);
            }
            break;
            case NativeTypes::Int:
            case NativeTypes::UnsignedInt:
            {
                auto v = JSTool::toInt(argv[i - 1]);
                if(!v) throw std::runtime_error("NativePoint.call(NativeTypes::UnsignedInt(Int))解析失败");
                dcArgInt(thi->m_vm, *v);
            }
            break;
            case NativeTypes::Long:
            case NativeTypes::UnsignedLong:
            {
                auto v = JSTool::toInt(argv[i - 1]);
                if(!v) throw std::runtime_error("NativePoint.call(NativeTypes::UnsignedLong(Long))解析失败");
                dcArgLong(thi->m_vm, *v);
            }
            break;
            case NativeTypes::LongLong:
            case NativeTypes::UnsignedLongLong:
            {
                auto v = JSTool::toInt64(argv[i - 1]);
                if(!v) throw std::runtime_error("NativePoint.call(NativeTypes::UnsignedLongLong(LongLong))解析失败");
                dcArgLongLong(thi->m_vm, *v);
            }
            break;
            case NativeTypes::Float:
            {
                auto v = JSTool::toFloat(argv[i - 1]);
                if(!v) throw std::runtime_error("NativePoint.call(NativeTypes::Float)解析失败，或者值超出范围");
                dcArgFloat(thi->m_vm, *v);
            }
            break;
            case NativeTypes::Double:
            {
                auto v = JSTool::toDouble(argv[i - 1]);
                if(!v) throw std::runtime_error("NativePoint.call(NativeTypes::Double)解析失败");
                dcArgDouble(thi->m_vm, *v);
            }
            break;
            case NativeTypes::Pointer:
            {
                nativePointClass* thii = (nativePointClass*)JS_GetOpaque(argv[i - 1], nativePointClass::id);
                dcArgPointer(thi->m_vm, (thii == nullptr) ? 0 : (void*)(thii->get()));
            }
            break;
            default:
                spdlog::error("函数参数类型列表中出现非法类型，i:{}", i);
                break;
            }
        }

        // 调用
        switch(thi->m_agreeOn[0]) {
        case NativeTypes::Bool:
        {
            auto v = dcCallBool(thi->m_vm, ori);
            ret = JS_NewBool(ctx, v);
        }
        break;
        case NativeTypes::Char:
        case NativeTypes::UnsignedChar:
        {
            auto v = dcCallChar(thi->m_vm, ori);
            ret = JS_NewInt32(ctx, v);
        }
        break;
        case NativeTypes::Short:
        case NativeTypes::UnsignedShort:
        {
            auto v = dcCallShort(thi->m_vm, ori);
            ret = JS_NewInt32(ctx, v);
        }
        break;
        case NativeTypes::Int:
        case NativeTypes::UnsignedInt:
        {
            auto v = dcCallInt(thi->m_vm, ori);
            ret = JS_NewInt32(ctx, v);
        }
        break;
        case NativeTypes::Long:
        case NativeTypes::UnsignedLong:
        {
            auto v = dcCallLong(thi->m_vm, ori);
            ret = JS_NewInt32(ctx, v);
        }
        break;
        case NativeTypes::LongLong:
        case NativeTypes::UnsignedLongLong:
        {
            auto v = dcCallLongLong(thi->m_vm, ori);
            ret = JS_NewInt64(ctx, v);
        }
        break;
        case NativeTypes::Float:
        {
            auto v = dcCallFloat(thi->m_vm, ori);
            ret = JS_NewFloat64(ctx, v);
        }
        break;
        case NativeTypes::Double:
        {
            auto v = dcCallDouble(thi->m_vm, ori);
            ret = JS_NewFloat64(ctx, v);
        }
        break;
        case NativeTypes::Pointer:
        {
            auto v = dcCallPointer(thi->m_vm, ori);
            ret = nativePointClass::newNativePoint((uintptr_t)v);
        }
        break;
        case NativeTypes::Void:
        {
            dcCallVoid(thi->m_vm, ori);
        }
        break;
        default:
            break;
        }
        dcReset(thi->m_vm);
    }
    catch(std::runtime_error& re) {
        spdlog::error(re.what());
        spdlog::error("错误发生在-函数：{}，文件：{}", __FUNCTION__, __FILE__);
        throw re;
    }
    catch(std::exception& e) {
        spdlog::error(e.what());
        spdlog::error("错误发生在-函数：{}，文件：{}", __FUNCTION__, __FILE__);
    }


    return ret;
}

JSValue nativePointClass::toNumber(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
    nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
    return JS_NewInt64(ctx, thi->m_ptr);
}
