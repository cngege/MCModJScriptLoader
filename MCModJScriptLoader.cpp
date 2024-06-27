// MCModJScriptLoader.cpp: 定义应用程序的入口点。
//
#include <iostream>
#include <fstream>
#include <Shlobj.h>
#include <filesystem>

#include <iostream>

#include "imgui_kiero/kiero.h"
#include "hook/HookImgui.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace fs = std::filesystem;

static auto start(HMODULE hModule) -> void {
    const char* local = getenv("LOCALAPPDATA");//C:\Users\CNGEGE\AppData\Local\Packages\microsoft.minecraftuwp_8wekyb3d8bbwe\AC
    fs::path moduleDir = std::string(local) + "\\..\\RoamingState\\JSRunner";
    if (!fs::exists(moduleDir) || !fs::is_directory(moduleDir)) {
        fs::create_directories(moduleDir);
    }
    auto file_logger = spdlog::basic_logger_mt("basic_logger", (moduleDir / "app.log").string());
    spdlog::set_default_logger(file_logger);
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);  // 日志保存等级
    spdlog::info("完工撒花..");

    // 拿到要Hook的关键函数的指针
    //ptr = findSig("0F B6 ? 88 ? 0F B6 42 01 88 41 01 0F");
    //_ASSERT(ptr);

    // 拿到参数到 玩家行为控制系统的指针
    //auto _offset = FindSignatureRelay(ptr, "0F 10 42", 32);
    //_ASSERT(_offset);
    //offset = (int)*reinterpret_cast<byte*>(_offset + 3);

    // 创建&开启Hook

    // 开启IMGUI HOOK


    //info = CreateHook((void*)ptr, (void*)&LockControlInputCallBack);
    //status = EnableHook(&info);
    ImguiHooks::InitImgui();

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
    }

    return TRUE;
}
