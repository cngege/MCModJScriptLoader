#ifndef CLIENT_MODMANAGER_H
#define CLIENT_MODMANAGER_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class ModManager {
public:
	static ModManager* getInstance();

	static auto getMCBase() -> uintptr_t;
	static auto getMCRunnerPath() -> fs::path;
	static auto getMCFolderPath() -> fs::path;
	
public:
	auto setModulePath(std::string path) -> void;
	auto setModulePath(std::filesystem::path path) -> void;
	auto getModulePath() const-> const std::filesystem::path&;

	auto setImConfigPath(std::filesystem::path path) -> void;
	auto getImConfigPath()const-> const std::filesystem::path&;

	auto pathCreate(const std::string& path) const-> bool;
	auto getPath(const std::string& path) const-> fs::path;

private:
	std::filesystem::path m_moduleDir{};
	std::filesystem::path m_ImConfigIni{};
};

#endif // !CLIENT_MODMANAGER_H

