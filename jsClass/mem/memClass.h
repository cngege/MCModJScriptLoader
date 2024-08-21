#pragma once

#include "quickjs/quickjs.h"

class memClass {
public:
	static void Reg(JSContext* ctx);
	static void Dispose();

	template <class T>
	static void setValue(uintptr_t, T);

	template <class T>
	static T getValue(uintptr_t);

private:
	static JSValue constructor(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);

	static JSValue findSig(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue findSigRelay(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue setBoolValue(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue getBoolValue(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);

};


