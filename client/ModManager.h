#ifndef CLIENT_MODMANAGER_H
#define CLIENT_MODMANAGER_H

#include <string>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <shared_mutex>

#include "nlohmann/Json.hpp"

namespace fs = std::filesystem;

class ModManager {
private:
    std::unordered_map<std::string, fs::path> AllPath = {};
    std::shared_mutex rw_mtx_pathList;
public:
    static ModManager* getInstance();

    static auto getModuleBase(const std::string& module) -> uintptr_t;
    static auto getMCBase() -> uintptr_t;
    static auto getCoreWindow() -> uintptr_t;
    static auto getMCRunnerPath() -> fs::path;
    static auto getMCFolderPath() -> fs::path;
    
public:
    auto setModulePath(std::string path) -> void;
    auto setModulePath(std::filesystem::path path) -> void;
    auto getModulePath() const-> const std::filesystem::path&;

    auto setImConfigPath(std::filesystem::path path) -> void;
    auto getImConfigPath()const-> const std::filesystem::path&;

    auto setOtherPath(std::string name, fs::path path)->void;
    auto getOtherPath(std::string name) ->fs::path;

    auto setImLogPath(std::filesystem::path path) -> void;
    auto getImLogPath()const-> const std::filesystem::path&;

    auto pathCreate(const std::string& path) const-> bool;
    auto getPath(const std::string& path) const-> fs::path;

    auto runinModThread(std::function<void()>)-> void;
    auto stopSign() -> void;
    auto loopback() const -> void;

    // 收到异常后打印并停止dll
    auto trySafeExceptions(const std::exception&) -> void;

    auto registerImGuiMouseHandle() -> void;
    auto disableMod(uintptr_t) -> void;

    //auto readConfig() -> nlohmann::json;
    //auto writeConfig(nlohmann::json config) -> bool;

    auto getImGuiConsoleWindow() -> struct ExampleAppConsole*;
private:
    bool modState = 0;
    std::filesystem::path m_moduleDir{};
    std::filesystem::path m_ImConfigIni{};
    std::filesystem::path m_ImLogIni{};
};

#endif // !CLIENT_MODMANAGER_H

