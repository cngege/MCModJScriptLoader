#include "quickjs/quickjs.h"

class JSManager {
public:
	static JSManager* getInstance();

public:
	auto setctx(JSContext* ctx) -> void;
	auto getctx() -> JSContext* const;


private:
	JSContext* m_ctx = nullptr;
};