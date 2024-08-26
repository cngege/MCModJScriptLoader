#pragma once

#include "quickjs/quickjs.h"

class memClass {
public:
	static void Reg();
	static void Dispose();
private:
	static JSValue constructor(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);

	static JSValue findSig(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue findSigRelay(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue setBoolValue(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue getBoolValue(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);

};


