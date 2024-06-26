// MCModJScriptLoader.cpp: 定义应用程序的入口点。
//
#include <iostream>
#include <Shlobj.h>
#include <filesystem>

#include "imgui_kiero/kiero.h"
#include "hook/HookImgui.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

static auto start(HMODULE hModule) -> void {
    const char* local = getenv("LOCALAPPDATA");
    std::string moduleDir = std::string(local) + "\\Packages\\Microsoft.MinecraftUWP_8wekyb3d8bbwe\\RoamingState\\JSRunner";
    moduleDir = std::string(local) + "\\Packages\\Microsoft.MinecraftUWP_8wekyb3d8bbwe\\RoamingState";
    if (!std::filesystem::exists(moduleDir)) {
        std::filesystem::create_directories(moduleDir);
    }
    std::filesystem::create_directories(moduleDir);
    auto file_logger = spdlog::basic_logger_mt("basic_logger", moduleDir + "\\app.log");
    spdlog::set_default_logger(file_logger);
    spdlog::set_level(spdlog::level::info);
    spdlog::info("debuger..");

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
