#pragma once
#include "quickjs/quickjs.h"
#include <string>

class JSManager {
public:
	static JSManager* getInstance();

public:
	auto setctx(JSContext* ctx) -> void;
	auto getctx() -> JSContext* const;

	auto loadNativeModule() -> void;
	auto loadModuleFromFile(const std::string&) -> JSModuleDef*;
	auto loadJSFromFoder(const std::string& = "") -> void;

private:
	JSContext* m_ctx = nullptr;
};