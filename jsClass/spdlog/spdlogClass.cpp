#include "spdlogClass.h"
#include "spdlog/spdlog.h"
#include "../JSManager.h"


namespace {
	static JSClassID id;
	static JSClassDef _spdlogClass = {
		.class_name{"spdlog"},
		.finalizer{[](JSRuntime* rt, JSValue val) {
				auto spdlog = (spdlogClass*)JS_GetOpaque(val, id);
				delete spdlog;
			}
		}
	};
}



void spdlogClass::Reg() {
	JSContext* ctx = JSManager::getInstance()->getctx();
	auto rt = JS_GetRuntime(ctx);
	JS_NewClassID(&id);
	JS_NewClass(rt, id, &_spdlogClass);

	JSValue protoInstance = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, protoInstance, "info", JS_NewCFunction(ctx, spdlogClass::info, "info", 1));
	JS_SetPropertyStr(ctx, protoInstance, "warn", JS_NewCFunction(ctx, spdlogClass::warn, "warn", 1));
	JS_SetPropertyStr(ctx, protoInstance, "error", JS_NewCFunction(ctx, spdlogClass::error, "error", 1));
	JS_SetPropertyStr(ctx, protoInstance, "debug", JS_NewCFunction(ctx, spdlogClass::debug, "debug", 1));

	JSValue ctroInstance = JS_NewCFunction2(ctx, &spdlogClass::constructor, _spdlogClass.class_name, 0, JS_CFUNC_constructor, 0);
	JS_SetConstructor(ctx, ctroInstance, protoInstance);
	JS_SetClassProto(ctx, id, protoInstance);

	JSValue global_obj = JS_GetGlobalObject(ctx);
	JS_SetPropertyStr(ctx, global_obj, _spdlogClass.class_name, ctroInstance);
	JS_FreeValue(ctx, global_obj);
}

void spdlogClass::Dispose() {
	
}

JSValue spdlogClass::constructor(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	JSValue obj = JS_NewObjectClass(ctx, id);
	auto self = new spdlogClass;
	JS_SetOpaque(obj, self);
	return obj;
}

JSValue spdlogClass::info(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
	std::string allstr;
	for(int i = 0; i < argc; i++) {
		auto str = JS_ToCString(ctx, JS_ToString(ctx, argv[i]));
		if(i) {
			allstr += " ";
		}
		allstr += str;
		JS_FreeCString(ctx, str);
	}
	spdlog::info(allstr);
	return JS_UNINITIALIZED;
}

JSValue spdlogClass::warn(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
	std::string allstr;
	for(int i = 0; i < argc; i++) {
		auto str = JS_ToCString(ctx, JS_ToString(ctx, argv[i]));
		if(i) {
			allstr += " ";
		}
		allstr += str;
		JS_FreeCString(ctx, str);
	}
	spdlog::warn(allstr);
	return JS_UNINITIALIZED;
}

JSValue spdlogClass::error(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
	std::string allstr;
	for(int i = 0; i < argc; i++) {
		auto str = JS_ToCString(ctx, JS_ToString(ctx, argv[i]));
		if(i) {
			allstr += " ";
		}
		allstr += str;
		JS_FreeCString(ctx, str);
	}
	spdlog::error(allstr);
	return JS_UNINITIALIZED;
}

JSValue spdlogClass::debug(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
	std::string allstr;
	for(int i = 0; i < argc; i++) {
		auto str = JS_ToCString(ctx, JS_ToString(ctx, argv[i]));
		if(i) {
			allstr += " ";
		}
		allstr += str;
		JS_FreeCString(ctx, str);
	}
	spdlog::debug(allstr);
	return JS_UNINITIALIZED;
}

