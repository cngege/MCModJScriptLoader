#include "JSManager.h"
#include <fstream>
#include <iostream>
#include "../client/ModManager.h"
#include "spdlog/spdlog.h"



static std::string JS_ErrorStackCheck(JSContext* ctx) {
	JSValue err = JS_GetException(ctx);
	bool isError = JS_IsError(ctx, err);
	std::string result_str;
	if(isError) {
		JSValue name = JS_GetPropertyStr(ctx, err, "name");
		const char* errorname_str = JS_ToCString(ctx, name);
		result_str = (errorname_str) ? errorname_str : "<unknown error name>";
		result_str += " - ";
		JS_FreeCString(ctx, errorname_str);
		JS_FreeValue(ctx, name);
		JSValue message = JS_GetPropertyStr(ctx, err, "message");
		const char* message_str = JS_ToCString(ctx, message);
		result_str += (message_str) ? std::string("\"") + message_str + "\"" : "<no message>";
		result_str += ":: \n";
		JS_FreeCString(ctx, message_str);
		JS_FreeValue(ctx, message);
		JSValue stack = JS_GetPropertyStr(ctx, err, "stack");
		if(!JS_IsUndefined(stack)) {
			const char* stack_str = JS_ToCString(ctx, stack);
			if(stack_str) {
				result_str += stack_str;
				JS_FreeCString(ctx, stack_str);
			}
		}
		JS_FreeValue(ctx, stack);
	}
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


auto JSManager::loadNativeModule() -> void {
	js_init_module_mem(m_ctx, "mem");


}

auto JSManager::loadModuleFromFile(const std::string& path) -> JSModuleDef* {
	// 接收到的就是完整路径
	// 检查文件是否存在
	if(!fs::exists(path)) {
		spdlog::error("加载模块时出错，没有找到文件");
		JS_ThrowReferenceError(m_ctx, "没有找到模块: %s", path.c_str());
		return nullptr;
	}
	
	// 加载模块
	std::ifstream jsfile(path);
	std::string content((std::istreambuf_iterator<char>(jsfile)), std::istreambuf_iterator<char>());
	JSValue val = JS_Eval(m_ctx, content.c_str(), content.size(), path.c_str(), JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
	if(JS_IsException(val)) {
		spdlog::error("JS Module Loader fail: {}", path);
		auto result_str = JS_ErrorStackCheck(m_ctx);
		spdlog::error(result_str);
		return nullptr;
	}
	JSModuleDef* module = (JSModuleDef*)JS_VALUE_GET_PTR(val);
	JS_FreeValue(m_ctx, val);
	return module;
}

// BaseFolder script, load All File
auto JSManager::loadJSFromFoder(const std::string& folder) -> void {
	for(const auto& entry : fs::directory_iterator(ModManager::getInstance()->getPath("script") / folder)) {
		if(entry.is_regular_file() && entry.path().extension() == ".js") {
			std::ifstream jsfile(entry.path());
			std::string content((std::istreambuf_iterator<char>(jsfile)), std::istreambuf_iterator<char>());
			spdlog::info("JS Loader: {}", entry.path().filename().string());
			JSValue val = JS_Eval(m_ctx, content.c_str(), content.size(), (fs::path(folder) / entry.path().filename()).string().c_str(), JS_EVAL_TYPE_MODULE);
			if(JS_IsException(val)) {
				spdlog::error("JS Loader fail: {}", entry.path().string());
				auto result_str = JS_ErrorStackCheck(m_ctx);
				spdlog::error(result_str);
			}
			JS_FreeValue(m_ctx, val);
		}
	}
}


