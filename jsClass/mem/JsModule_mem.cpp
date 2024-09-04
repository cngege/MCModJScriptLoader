#include "JsModule_mem.h"
#include <string>

#include "../../client/mem/mem.h"
#include "../JSManager.h"

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

static const JSCFunctionListEntry js_mem_funcs[] = {
    //JS_CFUNC_DEF("Get", 1, js_http_get_request),
	 { "findSig", JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, 0, {.func = { 1, JS_CFUNC_generic,{ .generic = js_findSig } } } },
	 { "findSigRelay", JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, 0, {.func = { 1, JS_CFUNC_generic,{ .generic = js_findSigRelay } } } },
	 { "setBoolValue", JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, 0, {.func = { 1, JS_CFUNC_generic,{ .generic = js_setBoolValue } } } },
	 { "getBoolValue", JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, 0, {.func = { 1, JS_CFUNC_generic,{ .generic = js_getBoolValue } } } },
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