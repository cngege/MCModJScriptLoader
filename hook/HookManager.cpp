#include "HookManager.h"
#include "spdlog/spdlog.h"
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

static std::shared_mutex map_lock_mutex;
static std::unordered_map<uintptr_t, std::pair<HookInformation, HookInstance>> hookInfoHash{};

auto HookManager::init()->void {

}

auto HookManager::addHook(uintptr_t ptr, void* fun) -> HookInstance* {
	std::shared_lock<std::shared_mutex> guard(map_lock_mutex);
	auto it = hookInfoHash.find(ptr);
	if(it != hookInfoHash.end()) {
		spdlog::warn("addHook 新增Hook失败,当前函数已被hook({}) - fromFunction {} - in {}", (const void*)ptr, __FUNCTION__, __LINE__);
		return nullptr;
	}
	else {
		hookInfoHash[ptr] = {
			CreateHook((void*)ptr, fun),
			HookInstance(ptr)
		};
		return &hookInfoHash[ptr].second;
	}
}

auto HookManager::enableHook(HookInstance& instance) -> int {
	std::shared_lock<std::shared_mutex> guard(map_lock_mutex);
	int ret = EnableHook(&hookInfoHash[instance.ptr()].first);
	instance.origin = hookInfoHash[instance.ptr()].first.Trampoline;
	return ret;
}

auto HookManager::disableHook(HookInstance& instance) -> int {
	std::shared_lock<std::shared_mutex> guard(map_lock_mutex);
	return DisableHook(&hookInfoHash[instance.ptr()].first);
}

auto HookManager::enableAllHook() -> void {
	std::shared_lock<std::shared_mutex> guard(map_lock_mutex);
	for(auto& item : hookInfoHash) {
		if(!item.second.first.Enabled) {
			if(!EnableHook(&item.second.first)) {
				spdlog::warn("EnableHook 开启Hook失败({}) - fromFunction {} - in {}",(const void*)item.first, __FUNCTION__, __LINE__);
			}
		}
	}
}

auto HookManager::disableAllHook() -> void {
	std::shared_lock<std::shared_mutex> guard(map_lock_mutex);
	for(auto& item : hookInfoHash) {
		if(item.second.first.Enabled) {
			if(!DisableHook(&item.second.first)) {
				spdlog::warn("DisableHook 关闭Hook失败({}) - fromFunction {} - in {}", (const void*)item.first, __FUNCTION__, __LINE__);
			}
		}
	}
}

auto HookManager::uninit() -> void {}

uintptr_t HookInstance::ptr() const {
	return m_ptr;
}

bool HookInstance::hook() {
	return HookManager::enableHook(*this);
}

bool HookInstance::unhook() {
	return HookManager::disableHook(*this);
}
