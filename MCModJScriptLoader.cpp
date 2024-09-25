// MCModJScriptLoader.cpp: 定义应用程序的入口点。
//

#include <fstream>
#include <iostream>

#include "hook/HookManager.h"
#include "client/ModManager.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/callback_sink.h"

#include "imgui_kiero/kiero.h"
#include "hook/HookImgui.h"

#include "client/mem/mem.h"
#include "client/utils/signcode.h"

#include "client/utils/assets_net.h"

#include "imgui/imgui_uwp_wndProc.h"

#include "quickjs/quickjs-libc.h"

#include "jsClass/JSManager.h"
#include "jsClass/spdlog/spdlogClass.h"
//#include "jsClass/mem/JsModule_mem.h"
#include "jsClass/hook/hookClass.h"
#include "jsClass/nativePoint/nativePointClass.h"

using MouseUpdata = void(__fastcall*)(__int64, char, char, __int16, __int16, __int16, __int16, char);
HookInstance* mouseupdate_info = nullptr;
static auto MouseUpdate(__int64 a1, char mousebutton, char isDown, __int16 mouseX, __int16 mouseY, __int16 relativeMovementX, __int16 relativeMovementY, char a8)->void {
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
				io.AddMouseButtonEvent(0, isDown);
				break;
			case 2:
				io.AddMouseButtonEvent(1, isDown);
				break;
			case 3:
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

	auto mouseupdatecall = (MouseUpdata)mouseupdate_info->origin;
	mouseupdatecall(a1, mousebutton, isDown, mouseX, mouseY, relativeMovementX, relativeMovementY, a8);
}



static JSModuleDef* js_module_loader_local(JSContext* ctx, const char* module_name, void* opaque) {
	if(std::string(module_name).ends_with(".js") || std::string(module_name).ends_with(".ts")) {
		return JSManager::getInstance()->loadModuleFromFile((ModManager::getInstance()->getPath("script") / module_name).string());
	}
	else {
		return js_module_loader(ctx, module_name, opaque);
	}
}

//BedrockLogOut
uintptr_t gamelogger_ori = 0;
HookInstance* gamelogger_info = nullptr;
static void BedrockLogOutHook(UINT type, const char* fmt, ...) {
	try {
		va_list va;
		va_start(va, fmt);
		char tn[1024];
		memset(tn, '\0', 1024);
		vsnprintf(tn, sizeof(tn) - 1, fmt, va);
		if(tn[strlen(tn) - 1] == '\n') {
			tn[strlen(tn) - 1] = '\0';
		}
		switch(type) {
		case 1:
			spdlog::debug(tn);
			break;
		case 2:
			spdlog::info(tn);
			break;
		case 8:
			spdlog::error(tn);
			break;
		case 4:
		default:
			spdlog::warn(tn);
			break;
		}
		va_end(va);
	}
	catch(...) {
		spdlog::error("游戏内部日志输出错误...");
	}
}


static JSRuntime* rt = nullptr;
static JSContext* ctx = nullptr;

static auto start(HMODULE hModule) -> void {
	char* localAppData = nullptr;
	size_t localsize = 0;
	_dupenv_s(&localAppData, &localsize, "LOCALAPPDATA");
	if(localAppData == nullptr) {
		throw std::runtime_error("取环境变量 LOCALAPPDATA 失败");
		return;
	}
    //const char* local = getenv("LOCALAPPDATA");//C:\Users\CNGEGE\AppData\Local\Packages\microsoft.minecraftuwp_8wekyb3d8bbwe\AC
	// 中文字体：https://ghproxy.cc/https://github.com/cngege/MCModJScriptLoader/releases/download/0.0.1/JNMYT.ttf
    fs::path moduleDir = std::string(localAppData) + "\\..\\RoamingState\\JSRunner";
	ModManager::getInstance()->setModulePath(moduleDir);

	ModManager::getInstance()->pathCreate("");
	ModManager::getInstance()->pathCreate("script");
	ModManager::getInstance()->pathCreate("script/module");
	ModManager::getInstance()->pathCreate("config");
	ModManager::getInstance()->pathCreate("Assets");
	ModManager::getInstance()->pathCreate("Assets/Fonts");

    if (fs::exists(moduleDir / "app.log")) {
        fs::remove(moduleDir / "app.log");
    }
	ModManager::getInstance()->setImConfigPath("config/imgui.ini");

	auto file_logger = spdlog::basic_logger_mt("MCModJScriptLoader", ModManager::getInstance()->getPath("app.log").string());
	file_logger->sinks().push_back(std::make_shared<spdlog::sinks::callback_sink_mt>([](const spdlog::details::log_msg& msg) {
		switch(msg.level) {
		case spdlog::level::info:
			GetImguiConsole()->AddLog("[info] %s", std::string(msg.payload.begin(), msg.payload.end()).c_str());
			break;
		case spdlog::level::warn:
			GetImguiConsole()->AddLog("[warn] %s", std::string(msg.payload.begin(), msg.payload.end()).c_str());
			break;
		case spdlog::level::err:
			GetImguiConsole()->AddLog("[error] %s", std::string(msg.payload.begin(), msg.payload.end()).c_str());
			break;
		case spdlog::level::debug:
			GetImguiConsole()->AddLog("[debug] %s", std::string(msg.payload.begin(), msg.payload.end()).c_str());
			break;
		case spdlog::level::trace:
			GetImguiConsole()->AddLog("[trace] %s", std::string(msg.payload.begin(), msg.payload.end()).c_str());
			break;
		case spdlog::level::critical:
			GetImguiConsole()->AddLog("[critical] %s", std::string(msg.payload.begin(), msg.payload.end()).c_str());
			break;
		default:
			break;
		}
	}));
    spdlog::set_default_logger(file_logger);

    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);  // 日志保存等级
    spdlog::info("日志部分完工撒花..");
	
	// httplib 下载字体文件
	http::downFont_JNMYT(ModManager::getInstance()->getPath("Assets/Fonts"));
	

    // 开启IMGUI HOOK
	{
		SignCode sign("MouseUpdate");
		sign << "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 44 0F";
		if(sign) {
			mouseupdate_info = HookManager::addHook(*sign, (void*)&MouseUpdate);
			mouseupdate_info->hook();
		}
		else {
			spdlog::warn("Mouse Hook fail. in {}", __FUNCTION__);
		}
		ImguiHooks::InitImgui();
	}


	// BedrockLogOut 函数定位 内部有字符串 "!!! ERROR: Unable to format log output message !!!"
	{
		SignCode sign("BedrockLogOut");
		sign << "48 89 54 24 ? 4C 89 44 24 ? 4C 89 4C 24 ? 55 53 56 57 41 56 41 57 48";
		sign.AddSignCall("15 ? ? ? ? 41 8B CE E8", 9);
		if(sign) {
			gamelogger_info = HookManager::addHook(*sign, (void*)&BedrockLogOutHook);
			gamelogger_info->hook();
		}
		else {
			spdlog::warn("gamelogger Hook fail. in {}", __FUNCTION__);
		}
	}

	//JS Runner
	rt = JS_NewRuntime();
	ctx = JS_NewContext(rt);
	js_std_init_handlers(rt);
	JSManager::getInstance()->setctx(ctx);

	// 开启BigNumber
	//JS_AddIntrinsicBigFloat(ctx);
	//JS_AddIntrinsicBigDecimal(ctx);
	//JS_AddIntrinsicOperators(ctx);
	//JS_EnableBignumExt(ctx, true);

	JS_SetModuleLoaderFunc(rt, nullptr, js_module_loader_local, nullptr);
	js_init_module_std(ctx, "std");
	js_init_module_os(ctx, "os");
	
	JSManager::getInstance()->loadNativeModule();

	spdlogClass::Reg();
	hookClass::Reg();
	nativePointClass::Reg();

	JSManager::getInstance()->loadJSFromFoder();
}

static auto stop()->void {
	//卸载Hook
	HookManager::disableAllHook();

	// 释放JS接口申请的资源
	spdlogClass::Dispose();
	hookClass::Dispose();
	nativePointClass::Dispose();

	// JS释放
	JS_FreeContext(ctx);
	JS_FreeRuntime(rt);

	// spdlog shutdown
	spdlog::shutdown();
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