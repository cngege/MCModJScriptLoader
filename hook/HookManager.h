#pragma once
#ifndef HOOK_HOOKMANAGER_H
#define HOOK_HOOKMANAGER_H

#include "LightHook/LightHook.h"


class HookInstance {
private:
	uintptr_t m_ptr = 0;
public:
	void* origin = nullptr;
public:
	HookInstance(){};
	HookInstance(uintptr_t ptr) : m_ptr(ptr) {};

	uintptr_t ptr() const;
	bool hook();
	bool unhook();
};

class HookManager
{
public:
	static auto init()->void;
	static auto addHook(uintptr_t ptr, void* fun) -> HookInstance*;

	static auto enableHook(HookInstance&) -> int;
	static auto disableHook(HookInstance&) -> int;
	static auto enableAllHook() -> void;
	static auto disableAllHook() -> void;
	static auto uninit() -> void;
private:

};
#endif   //HOOK_HOOK_H