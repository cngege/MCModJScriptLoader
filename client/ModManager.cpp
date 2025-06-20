﻿#include "ModManager.h"
#include <Windows.h>
#include "spdlog/spdlog.h"
#include "utils/signcode.h"
#include <shared_mutex>
#include <fstream>
#include "../jsClass/JSManager.h"
#include "imgui/imgui_uwp_wndProc.h"
#include "quickjs/quickjs-libc.h"
#include "imgui/appConsole.h"

ModManager* ModManager::getInstance() {
    static ModManager instance{};
    return &instance;
}

auto ModManager::getModuleBase(const std::string& module) -> uintptr_t {
    static auto cachedBase = (uintptr_t)GetModuleHandle(module.c_str());
    return cachedBase;
}

auto ModManager::getMCBase() -> uintptr_t {
    static auto cachedBase = (uintptr_t)GetModuleHandle("Minecraft.Windows.exe");
    return cachedBase;
}

// 这个暂时用不上，就算获取到了也得在CoreWindow的UI线程才能用,不如直接调用接口获取
auto ModManager::getCoreWindow() -> uintptr_t {
    static uintptr_t corewindow = 0;
    if(corewindow) return corewindow;

    //0F 10 1D ? ? ? ? 0F 11 84
    SignCode sign("Find CoreWindow Base for SignCode", true, true);
    sign.AddSignCall("0F 10 1D ? ? ? ? 0F 11 84", 3, [](uintptr_t p)->uintptr_t { return p + 8; });
    sign.AddSignCall("0F 11 05 ? ? ? ? 0F 10 44 24 ? 0F 11 0D", 3, [](uintptr_t p)->uintptr_t { return p + 8; });
    if(sign) {
        corewindow = *sign;
    }
    return corewindow;
}

auto ModManager::getMCRunnerPath() -> fs::path {
    static fs::path mcPath;
    if(mcPath.empty()) {
        TCHAR szPath[_MAX_PATH] = { 0 };
        GetModuleFileName((HMODULE)getMCBase(), szPath, _MAX_PATH - 1);
        mcPath = szPath;
    }
    return mcPath;
}

auto ModManager::getMCFolderPath() -> fs::path {
    return getMCRunnerPath().parent_path();
}

auto ModManager::setModulePath(std::string path) -> void {
    m_moduleDir = path;
}

auto ModManager::setModulePath(std::filesystem::path path) -> void {
    m_moduleDir = path;
}

auto ModManager::getModulePath() const -> const std::filesystem::path& {
    return m_moduleDir;
}

auto ModManager::setImConfigPath(std::filesystem::path path) -> void {
    m_ImConfigIni = m_moduleDir / path;
}

auto ModManager::getImConfigPath() const -> const std::filesystem::path& {
    return m_ImConfigIni;
}

auto ModManager::setOtherPath(std::string name, fs::path path) -> void {
    std::unique_lock<std::shared_mutex> lock(rw_mtx_pathList);
    AllPath[name] = m_moduleDir / path;
}

auto ModManager::getOtherPath(std::string name) -> fs::path {
    std::shared_lock<std::shared_mutex> lock(rw_mtx_pathList);
    auto it = AllPath.find(name);
    if(it != AllPath.end()) {
        return it->second;
    }
    return fs::path();
}

auto ModManager::setImLogPath(std::filesystem::path path) -> void {
    m_ImLogIni = m_moduleDir / path;
}

auto ModManager::getImLogPath() const -> const std::filesystem::path& {
    return m_ImLogIni;
}


auto ModManager::pathCreate(const std::string& path) const -> bool {
    if(path.empty()) {
        if(!fs::exists(m_moduleDir) || !fs::is_directory(m_moduleDir)) {
            return fs::create_directories(m_moduleDir);
        }
        return true;
    }

    fs::path _dir = m_moduleDir / path;

    if(!fs::exists(_dir) || !fs::is_directory(_dir)) {
        return fs::create_directories(_dir);
    }
    return true;
}

auto ModManager::getPath(const std::string& path) const -> fs::path {
    return m_moduleDir / path;
}

static std::vector<std::function<void()>> eventlist;
static std::shared_mutex rw_mtx_eventList;
// 在loop 循环期间,读取列表中的事件执行一次
auto ModManager::runinModThread(std::function<void()> e) -> void {
    std::unique_lock<std::shared_mutex> lock(rw_mtx_eventList);
    eventlist.push_back(e);
}

auto ModManager::stopSign() -> void {
    modState = true;
}


auto ModManager::loopback() const -> void {
    bool IsjsLoop = true;
    auto rt = JSManager::getInstance()->getrt();
    auto ctx = JSManager::getInstance()->getctx();
    int r = 0;
    int exception_count = 0;
    while(!modState) {
        if(exception_count > 100) {
            spdlog::warn("主循环中捕获到大量异常, 已跳出主循环");
            break;
        }
        try {
            {
                std::unique_lock<std::shared_mutex> lock(rw_mtx_eventList);

                try {
                    auto it = eventlist.begin();
                    while(it!=eventlist.end()) {
                        (*it)();
                        it = eventlist.erase(it);
                    }
                }
                catch(std::exception& err) {
                    spdlog::error("在事件列表中运行读取的一个事件时出错:{}", err.what());
                }

                //if(eventlist.size()) {
                //    try {
                //        auto it = eventlist.begin();
                //        (*it)();
                //        eventlist.erase(it);
                //    }
                //    catch(std::exception& err) {
                //        spdlog::error("在事件列表中运行读取的一个事件时出错:{}", err.what());
                //    }
                //}
            }
            if(IsjsLoop) {
                //if(JSManager::getInstance()->runstdLoop()) {
                //   IsjsLoop = false;
                //}
                r = JS_ExecutePendingJob(rt, &ctx);
                if(r < 0) {
                    int err = JS_HasException(ctx);
                    if(err) {
                        spdlog::error("std_loop ctx error: {}", JSManager::getInstance()->getErrorStack().c_str());
                        IsjsLoop = false;
                    }
                }
            }
            auto r2 = JSManager::getInstance()->runTaskQueue();
            if((r <= 0 || IsjsLoop == false) && !r2) {
                Sleep(100);
            }
            else {
                Sleep(1);
            }
        }
        catch(std::exception& e) {
            spdlog::error("{} 中发生异常: {}", __FUNCTION__, e.what());
            exception_count++;
        }
        catch(...) {
            spdlog::error("{} 中发生未知异常", __FUNCTION__);
            exception_count++;
        }

    }
}

auto ModManager::trySafeExceptions(const std::exception& e) -> void {
    if(!modState) {
        spdlog::error("{} -：{}", "trySafeExceptions", e.what());
        modState = true;
    }
}


auto ModManager::registerImGuiMouseHandle() -> void {
    registerCoreWindowEventHandle();
}

auto ModManager::disableMod(uintptr_t modhandle) -> void {
    unregisterCoreWindowEventHandle();
}

//auto ModManager::readConfig() -> nlohmann::json {
//    std::ifstream configFileR(ModManager::getInstance()->getOtherPath("ModConfig"), std::ios::in);
//    if(!configFileR.is_open()) return NULL;
//    nlohmann::json config = {};
//    configFileR >> config;
//    configFileR.close();
//    return config;
//}
//
//auto ModManager::writeConfig(nlohmann::json config) -> bool {
//    std::ofstream configFileW(ModManager::getInstance()->getOtherPath("ModConfig"));
//    if(!configFileW.is_open()) return false;
//    configFileW << std::setw(4) << config << std::endl;
//    configFileW.close();
//    return true;
//}

auto ModManager::getImGuiConsoleWindow() -> ExampleAppConsole* {
    return GetImguiConsole();
}


