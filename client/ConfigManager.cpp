#include "ConfigManager.h"
#include "ModManager.h"
#include "spdlog/spdlog.h"
#include <fstream>


auto ConfigManager::getInstance() -> ConfigManager* const {
    static ConfigManager instance;
    return &instance;
}

auto ConfigManager::loadConfig() -> json {
    auto path = ModManager::getInstance()->getOtherPath(m_configPathAlias);
    std::ifstream configFileR(path, std::ios::in);
    if(!configFileR.is_open()) {
        spdlog::warn("读取配置文件失败, 配置文件无法打开 in: {}, Line: {} file: {}", __FUNCDNAME__, __LINE__, path.string().c_str());
        return m_configData;
    }
    nlohmann::json config = {};
    configFileR >> config;
    configFileR.close();
    m_configData = config;
    return m_configData;
}

auto ConfigManager::readConfig() -> json& {
    if(m_configData != NULL) {
        return m_configData;
    }
    else {
        loadConfig();
        return m_configData;
    }
}

auto ConfigManager::writeConfig() -> bool {
    return writeConfig(m_configData);
}

auto ConfigManager::writeConfig(json& data) -> bool {
    std::ofstream configFileW(ModManager::getInstance()->getOtherPath(m_configPathAlias));
    if(!configFileW.is_open()) return false;
    configFileW << std::setw(4) << data << std::endl;
    configFileW.close();
    return true;
}

auto ConfigManager::isExists() -> bool const { return fs::exists(ModManager::getInstance()->getOtherPath(m_configPathAlias)); }

auto ConfigManager::getConfigPath() -> fs::path const { return ModManager::getInstance()->getOtherPath(m_configPathAlias); }

auto ConfigManager::asString() -> std::string {
    if(readConfig().empty()) return std::string();
    return readConfig().dump(4);
}
