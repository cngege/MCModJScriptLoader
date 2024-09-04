#pragma once

#include "quickjs/quickjs.h"

class spdlogClass {

public:
	static void Reg();
	static void Dispose();

private:
	static JSValue constructor(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);

	
	static JSValue info(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv);
	static JSValue warn(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv);
	static JSValue error(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv);
	static JSValue debug(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv);

	
};