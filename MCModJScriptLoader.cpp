// MCModJScriptLoader.cpp: 定义应用程序的入口点。
//
#include <fstream>
#include <iostream>
//#include <Shlobj.h>
#include <filesystem>

#include "LightHook/LightHook.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#include "imgui_kiero/kiero.h"
#include "hook/HookImgui.h"

#include "client/mem/mem.h"
//#include "imgui/imgui.h"
#include "imgui/imgui_uwp_wndProc.h"

#include "quickjs/quickjs-libc.h"

#include "jsClass/spdlog/spdlogClass.h"
#include "jsClass/mem/memClass.h"
#include "jsClass/hook/hookClass.h"

namespace fs = std::filesystem;

using MouseUpdata = void(__fastcall*)(__int64, char, char, __int16, __int16, __int16, __int16, char);
HookInformation mouseupdate_info;
auto MouseUpdate(__int64 a1, char mousebutton, char isDown, __int16 mouseX, __int16 mouseY, __int16 relativeMovementX, __int16 relativeMovementY, char a8)->void {
	//Game::MouseKeyDown[mousebutton] = isDown;
	//Game::GetModuleManager()->onMouseUpdate(mousebutton, isDown, mouseX, mouseY, relativeMovementX, relativeMovementY);

	try
	{
		if (ImGui::GetCurrentContext() != nullptr) {
			ImGuiMouseSource mouse_source = GetMouseSourceFromMessageExtraInfo();
			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseSourceEvent(mouse_source);
			switch (mousebutton) {
			case 1:
				//io.MouseDown[0] = isDown;
				io.AddMouseButtonEvent(0, isDown);
				break;
			case 2:
				//io.MouseDown[1] = isDown;
				io.AddMouseButtonEvent(1, isDown);
				break;
			case 3:
				//io.MouseDown[2] = isDown;
				io.AddMouseButtonEvent(2, isDown);
				break;
			case 4:
				//io.MouseWheel = isDown < 0 ? -0.5f : 0.5f; //For scrolling
				io.AddMouseWheelEvent(0.f, isDown < 0 ? -1.f : 1.f);
				break;
			default:
				io.AddMousePosEvent(mouseX, mouseY);
				break;
			}
			if (/*io.WantCaptureMouse && */io.WantCaptureMouseUnlessPopupClose)
				return;
		}
	}
	catch (const std::exception& ex)
	{
		spdlog::error("MouseUpdate 异常: {}", ex.what());
	}

	auto mouseupdatecall = (MouseUpdata)mouseupdate_info.Trampoline;
	mouseupdatecall(a1, mousebutton, isDown, mouseX, mouseY, relativeMovementX, relativeMovementY, a8);
}

/*
#include <vector>
auto MouseUpdate2(uintptr_t thi,...) -> void {
	std::vector<uintptr_t> argcs = {};
	va_list args;
	va_start(args, thi);
	for(int i = 0; i < 7; ++i) {
		argcs.push_back(va_arg(args, uintptr_t));
	}
	va_end(args);
	return MouseUpdate((__int64)thi,(char)argcs[0], (char)argcs[1], (__int16)argcs[2], (__int16)argcs[3], (__int16)argcs[4], (__int16)argcs[5], (char)argcs[6]);
}*/
static JSRuntime* rt = nullptr;
static JSContext* ctx = nullptr;

static auto start(HMODULE hModule) -> void {
	char* localAppData = nullptr;
	size_t localsize = 0;
	_dupenv_s(&localAppData, &localsize, "LOCALAPPDATA");
    //const char* local = getenv("LOCALAPPDATA");//C:\Users\CNGEGE\AppData\Local\Packages\microsoft.minecraftuwp_8wekyb3d8bbwe\AC
    fs::path moduleDir = std::string(localAppData) + "\\..\\RoamingState\\JSRunner";
    if (!fs::exists(moduleDir) || !fs::is_directory(moduleDir)) {
        fs::create_directories(moduleDir);
    }
    if (fs::exists(moduleDir / "app.log")) {
        fs::remove(moduleDir / "app.log");
    }
	auto scriptDir = moduleDir / "script";
	if (!fs::exists(scriptDir) || !fs::is_directory(scriptDir)) {
		fs::create_directories(scriptDir);
	}
    auto file_logger = spdlog::basic_logger_mt("MCModJScriptLoader", (moduleDir / "app.log").string());
    spdlog::set_default_logger(file_logger);
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);  // 日志保存等级
    spdlog::info("日志部分完工撒花..");
	
    // 拿到要Hook的关键函数的指针
    //ptr = findSig("0F B6 ? 88 ? 0F B6 42 01 88 41 01 0F");
    //_ASSERT(ptr);

    // 拿到参数到 玩家行为控制系统的指针
    //auto _offset = FindSignatureRelay(ptr, "0F 10 42", 32);
    //_ASSERT(_offset);
    //offset = (int)*reinterpret_cast<byte*>(_offset + 3);

    // 创建&开启Hook

    // 开启IMGUI HOOK
    auto ptr = Mem::findSig("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 44 0F");
	if (ptr) {
		mouseupdate_info = CreateHook((void*)ptr, (void*)&MouseUpdate);
		EnableHook(&mouseupdate_info);
	}
	else {
		spdlog::warn("Mouse Hook fail.");
	}

    //info = CreateHook((void*)ptr, (void*)&LockControlInputCallBack);
    //status = EnableHook(&info);
    ImguiHooks::InitImgui();

	//JS Runner
	rt = JS_NewRuntime();
	ctx = JS_NewContext(rt);
	js_std_init_handlers(rt);

	JS_SetModuleLoaderFunc(rt, nullptr, js_module_loader, nullptr);
	js_init_module_std(ctx, "std");
	js_init_module_os(ctx, "os");

	spdlogClass::Reg(ctx);
	memClass::Reg(ctx);
	hookClass::Reg(ctx);

	for (const auto& entry : fs::directory_iterator(scriptDir)) {
		if (entry.is_regular_file() && entry.path().extension() == ".js") {
			std::ifstream jsfile(entry.path());
			std::string content((std::istreambuf_iterator<char>(jsfile)), std::istreambuf_iterator<char>());
			spdlog::info("JS Loader: {}", entry.path().string());
			JSValue val = JS_Eval(ctx, content.c_str(), content.size(), entry.path().filename().string().c_str(), JS_EVAL_TYPE_MODULE);
			if(JS_IsException(val)) {
				spdlog::error("JS Loader fail: {}", entry.path().string());

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
				spdlog::error(result_str);
			}
			JS_FreeValue(ctx, val);
		}
	}

	//JS_FreeContext(ctx);
	//JS_FreeRuntime(rt);
}

static auto stop()->void {
	// TODO: 卸载Hook

	// JS释放
	//JS_FreeContext(ctx);
	//JS_FreeRuntime(rt);
}

// Dll入口函数
auto APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) -> BOOL {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)start, hModule, NULL, nullptr);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        /*if (status) {
            // 反注入则关闭Hook
            DisableHook(&info);
        }*/
		stop();
    }

    return TRUE;
}



/*
* https://cloud.tencent.com/developer/article/1879884
* https://blog.csdn.net/qq_36291960/article/details/109048424
JS_ThrowSyntaxError  解析/编译期间发现的语法错误
JS_ThrowTypeError 对错误的数据类型执行操作时会发生TypeError
JS_ThrowReferenceError   引用错误 引用一个不存在的变量时发生的错误 如果我们调用的是一个已经存在的变量的一个不存在的属性，则不会抛出 ReferenceError，因为变量本身已经在存储中了，调用它不存在的属性只会是未定义状态，也就是 undefined:
JS_ThrowRangeError  边界错误 表示超出有效范围时发生的异常，主要的有以下几种情况：数组长度为负数或超长 数字类型的方法参数超出预定义范围 函数堆栈调用超过最大值
JS_ThrowOutOfMemory 内存不足相关
JS_ThrowInternalError  该错误在JS引擎内部发生，特别是当它有太多数据要处理并且堆栈增长超过其关键限制时。当JS引擎被太多的递归，太多的切换情况等淹没时，就会发生这种情况

*/