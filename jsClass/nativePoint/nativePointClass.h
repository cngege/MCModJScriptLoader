﻿#pragma once
#include "quickjs/quickjs.h"
#include <vector>

struct DCCallVM_;
enum NativeTypes
{
	Void = 0,
	Bool,
	Char,
	UnsignedChar,
	Short,
	UnsignedShort,
	Int,
	UnsignedInt,
	Long,
	UnsignedLong,
	LongLong,
	UnsignedLongLong,
	Float,
	Double,
	Pointer
};

class nativePointClass {
public:
	static void Reg();
	static void Dispose();
	/**
	 * @brief 从nativePointClass指针到nativePoint的JS对象
	 * @param  
	 * @return 
	 */
	static JSValue FromPtr(uintptr_t);
	/**
	 * @brief 从任意指针到nativePoint的JS对象
	 * @param  
	 * @return 
	 */
	static JSValue newNativePoint(uintptr_t);
	static JSValue newNativePoint(uintptr_t, std::vector<NativeTypes>);

public:
	nativePointClass(uintptr_t);
	~nativePointClass();
	uintptr_t get();

private:
	static JSValue constructor(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);

	static JSValue offset(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue setAgree(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue setbool(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue getbool(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue setint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue getint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue setlong(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue getlong(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue setfloat(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue getfloat(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue setdouble(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue getdouble(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue setpoint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue getpoint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);


	static JSValue call(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue toNumber(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);

private:
	DCCallVM_* m_vm = nullptr;
	uintptr_t m_ptr = 0;

	std::vector<NativeTypes> m_agreeOn{};

public:
	static JSClassID id;
};