#include "ModManager.h"
#include <Windows.h>



ModManager* ModManager::getInstance() {
	static ModManager instance{};
	return &instance;
}

auto ModManager::getMCBase() -> uintptr_t {
	static auto cachedBase = (uintptr_t)GetModuleHandle("Minecraft.Windows.exe");
	return cachedBase;
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


