#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "nlohmann/Json.hpp"
#include <string>

namespace fs = std::filesystem;
using json = nlohmann::json;

class ConfigManager {
private:
    /**
     * @brief 配置文件路径对于的别名
     */
    std::string m_configPathAlias = "ModConfig";
    /**
     * @brief 保留在内存中的配置数据
     */
    json m_configData;
public:
    static auto getInstance() -> ConfigManager* const;
    /**
     * @brief 从文件中读取配置
     * @return 
     */
    auto loadConfig() -> json;
    /**
     * @brief 从内存中读取配置,没有则从文件中读取，并保存
     * @return 
     */
    auto readConfig() -> json&;
    /**
     * @brief 使用内存保留区的配置写入到文件
     * @return 
     */
    auto writeConfig() -> bool;
    /**
     * @brief 使用指定的配置写入到文件
     * @param data 
     * @return 
     */
    auto writeConfig(json& data) -> bool;
    /**
     * @brief 配置文件本地是否存在
     * @return 
     */
    auto isExists() -> bool const ;
    /**
     * @brief 采用别名的方式设置配置文件路径
     * @param alias 别名, 此名必须要在ModManager中设置对于的路径
     * @return 
     */
    auto setConfigPathByAlias(std::string alias) { m_configPathAlias = alias; };
    /**
     * @brief 获取配置文件对于的路径
     * @return 
     */
    auto getConfigPath() -> fs::path const ;
    /**
     * @brief 配置数据转为字符串
     * @return 
     */
    auto asString() -> std::string;

};

#endif // !CONFIGMANAGER_H
