#include "memClass.h"
#include "../../client/mem/mem.h"
#include "../JSManager.h"
#include <string>

namespace {
	static JSClassID id;
	static JSClassDef _memClass = {
		.class_name{"mem"},
		.finalizer{[](JSRuntime* rt, JSValue val) {
				auto mem = (memClass*)JS_GetOpaque(val, id);
				delete mem;
			}
		}
	};
}

void memClass::Reg() {
	JSContext* ctx = JSManager::getInstance().getctx();
	JSValue global_obj = JS_GetGlobalObject(ctx);
	JSValue mem = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, global_obj, "mem", mem);

	JS_SetPropertyStr(ctx, mem, "findSig",
					  JS_NewCFunction(ctx, memClass::findSig, "findSig", 1));
	JS_SetPropertyStr(ctx, mem, "findSigRelay",
					  JS_NewCFunction(ctx, memClass::findSigRelay, "findSigRelay", 3));
	JS_SetPropertyStr(ctx, mem, "setBool",
					  JS_NewCFunction(ctx, memClass::setBoolValue, "setBool", 2));
	JS_SetPropertyStr(ctx, mem, "getBool",
					  JS_NewCFunction(ctx, memClass::getBoolValue, "getBool", 1));
}

void memClass::Dispose() {

}


JSValue memClass::constructor(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	JSValue obj = JS_NewObjectClass(ctx, id);
	auto self = new memClass;
	JS_SetOpaque(obj, self);
	return obj;
}

JSValue memClass::findSig(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
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

// 在地址后进行特征查找直到超出规定范围
JSValue memClass::findSigRelay(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
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

JSValue memClass::setBoolValue(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
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

JSValue memClass::getBoolValue(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
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
