#include "JSManager.h"

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


