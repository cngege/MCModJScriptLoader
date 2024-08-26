#pragma once

#include <string>
#include "quickjs/quickjs.h"
#include "LightHook/LightHook.h"


enum NativeTypes;
struct NativeUserData;
class HookInstance;

class hookClass {
public:
	static void Reg();
	static void Dispose();
	static char getTypeSignature(NativeTypes type);

private:
	static JSValue constructor(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue hook(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);
	static JSValue unhook(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv);

public:
	hookClass();
	~hookClass();


private:
	const std::string signature();
	

private:
	//JSValue m_hookFun;
	/**
	 * @brief 调用约定
	 */
	//std::vector<NativeTypes> m_agreeOn;
	HookInstance* m_hookinfo = nullptr;
	NativeUserData* m_userData = nullptr;
	JSValue origin{};
};




// hook 和 unhook 的机制