#include "JsModule_mem.h"
#include <string>

#include "../../client/mem/mem.h"
#include "../JSManager.h"
#include "../nativePoint/nativePointClass.h"
#include "../client/utils/hmath.h"

static JSValue js_findSig(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    std::string module = "Minecraft.Windows.exe";
    std::string signCode = "";
    if(argc >= 1) {
        if(argc >= 2) {
            auto str = JS_ToCString(ctx, JS_ToString(ctx, argv[1]));
            module = std::string(str);
            JS_FreeCString(ctx, str);
        }
        auto str = JS_ToCString(ctx, JS_ToString(ctx, argv[0]));
        auto ptr = Mem::findSig(str, module.c_str());
        JS_FreeCString(ctx, str);
        return JS_NewInt64(ctx, ptr);
        //return JS_NewBigInt64(ctx, ptr);
    }
    return JS_ThrowTypeError(ctx, "函数最少需要1个参数，当前参数个数：%d", argc);
}

static JSValue js_findSigRelay(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if(argc < 3) {
        return JS_ThrowTypeError(ctx, "函数最少需要3个参数，当前参数个数：%d", argc);
    }
    int64_t ptr;
    if(!JS_IsNumber(argv[0]) || JS_ToInt64(ctx, &ptr, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为指针，类型应为Number");
    }
    auto signstr = JS_ToCString(ctx, JS_ToString(ctx, argv[1]));
    int32_t range;
    if(JS_ToInt32(ctx, &range, argv[2]) < 0) {
        JS_FreeCString(ctx, signstr);
        return JS_ThrowTypeError(ctx, "参数三应为整数，类型应为Number");
    }
    auto retptr = Mem::findSigRelay(ptr, signstr, range);
    JS_FreeCString(ctx, signstr);
    return JS_NewInt64(ctx, retptr);
}

static JSValue js_getBase(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    int64_t ptr = 0;
    if(argc >= 1) {
        if(JS_IsString(argv[0])) {
            const char* module = JS_ToCString(ctx, argv[0]);
            ptr = Mem::getBase(module);
        }
        else {
            return JS_ThrowTypeError(ctx, "参数一传参时应传递String");
        }
    }
    else {
        ptr = Mem::getBase();
    }
    return JS_NewInt64(ctx, ptr);;
}

static JSValue js_setBoolValue(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "函数需要2个参数，当前参数个数：%d", argc);
    }
    int64_t ptr;
    if(!JS_IsNumber(argv[0]) || JS_ToInt64(ctx, &ptr, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为指针，类型应为Number");
    }
    if(!JS_IsBool(argv[1])) {
        return JS_ThrowTypeError(ctx, "参数二应为布尔值，类型应为Boolean");
    }
    int v = JS_ToBool(ctx, argv[1]);
    if(v < 0) {
        return JS_ThrowTypeError(ctx, "参数二转为布尔值失败");
    }
    Mem::setValue<bool>((uintptr_t)ptr, (bool)v);

    return JS_UNINITIALIZED;
}


static JSValue js_getBoolValue(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
    }
    int64_t ptr;
    if(!JS_IsNumber(argv[0]) || JS_ToInt64(ctx, &ptr, argv[0]) < 0) {
        return JS_ThrowTypeError(ctx, "参数一应为指针，类型应为Number");
    }
    if(ptr <= 0) {
        return JS_ThrowTypeError(ctx, "参数一指针不能小于等于0");
    }
    return JS_NewBool(ctx, Mem::getValue<bool>((uintptr_t)ptr));
}

static JSValue js_WorldToScreen(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if(argc < 5) return JS_ThrowTypeError(ctx, "函数需要5个参数[glmatrixf*, vec3-ori, vec3-pos,vec2-fov,vec2-screensize]，当前参数个数：%d", argc);
    auto _NativePoint = (nativePointClass*)JS_GetOpaque(argv[0], nativePointClass::id);
    if(_NativePoint == nullptr) return JS_NULL;

    glmatrixf* glm = (glmatrixf*)_NativePoint->get();
    auto ori = JSTool::getPropXYZ(argv[1]);
    if(!ori) return JS_ThrowTypeError(ctx, "参数2应包含属性 x,y,z");

    auto pos = JSTool::getPropXYZ(argv[2]);
    if(!pos) return JS_ThrowTypeError(ctx, "参数3应包含属性 x,y,z");

    auto fov = JSTool::getPropXY(argv[3]);
    if(!fov) return JS_ThrowTypeError(ctx, "参数4应包含属性 x,y");

    auto screensize = JSTool::getPropXY(argv[4]);
    if(!screensize) return JS_ThrowTypeError(ctx, "参数5应包含属性 x,y");
    
    vec2_t out = {};
    if(glm->correct().OWorldToScreen({ ori->at(0),ori->at(1) ,ori->at(2) }, { pos->at(0),pos->at(1) ,pos->at(2) }, out, { fov->at(0),fov->at(1) }, { screensize->at(0),screensize->at(1) })) {
        auto o = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, o, "x", JS_NewFloat64(ctx, out.x));
        JS_SetPropertyStr(ctx, o, "y", JS_NewFloat64(ctx, out.y));
        return o;
    }
    else {
        return JS_NULL;
    }
}

static const JSCFunctionListEntry js_mem_funcs[] = {
    JS_CFUNC_DEF2("查找特征码", 1, js_findSig),
    JS_CFUNC_DEF2("二次查找特征码", 1, js_findSigRelay),
    JS_CFUNC_DEF2("获取基址", 0, js_getBase),
    JS_CFUNC_DEF2("设置内存Bool值", 2, js_setBoolValue),
    JS_CFUNC_DEF2("读取内存Bool值", 2, js_getBoolValue),
    JS_CFUNC_DEF2("世界位置转屏幕位置", 2, js_WorldToScreen),
};

static int js_mem_init(JSContext* ctx, JSModuleDef* m) {
    return JS_SetModuleExportList(ctx, m, js_mem_funcs, _countof(js_mem_funcs));
}

JSModuleDef* js_init_module_mem(JSContext* ctx, const char* module_name) {
    JSModuleDef* m = JS_NewCModule(ctx, module_name, js_mem_init);
    if(!m)
        return NULL;
    JS_AddModuleExportList(ctx, m, js_mem_funcs, _countof(js_mem_funcs));
    return m;
}