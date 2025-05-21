#include "ModManager.h"
#include <Windows.h>
#include "spdlog/spdlog.h"
#include "utils/signcode.h"
#include <shared_mutex>


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
    SignCode sign("Find CoreWindow Base for SignCode");
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
    while(!modState) {
        {
            std::unique_lock<std::shared_mutex> lock(rw_mtx_eventList);
            if(eventlist.size()) {
                try {
                    auto it = eventlist.begin();
                    (*it)();
                    eventlist.erase(it);
                }
                catch(std::exception& err) {
                    spdlog::error("在事件列表中运行读取的一个事件时出错:{}", err.what());
                }
            }
        }
        Sleep(100);
    }
}

auto ModManager::trySafeExceptions(const std::exception& e) -> void {
    if(!modState) {
        spdlog::error("{} -：{}", "trySafeExceptions", e.what());
        modState = true;
    }
}


auto ModManager::disableMod(uintptr_t modhandle) -> void {
    
}


