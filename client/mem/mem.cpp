#include "mem.h"

#include <Windows.h>	//GetModuleHandleA
#include <vector>
#include <optional>
#include <algorithm>	//std::search

auto Mem::findSig(const char* signature, const char* modulename) -> uintptr_t
{
	static auto pattern_to_byte = [](const char* pattern) {

		auto bytes = std::vector<std::optional<uint8_t>>{};
		auto start = const_cast<char*>(pattern);
		auto end = const_cast<char*>(pattern) + strlen(pattern);
		bytes.reserve(strlen(pattern) / 2);

		for (auto current = start; current < end; ++current) {
			if (*current == '?') {
				++current;
				if (*current == '?')
					++current;
				bytes.push_back(std::nullopt);
			}
			else bytes.push_back((uint8_t)strtoul(current, &current, 16));
		}
		return bytes;

		};

	auto gameModule = (unsigned long long)(GetModuleHandleA(modulename));
	auto* const scanBytes = reinterpret_cast<uint8_t*>(gameModule);
	auto* const dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(gameModule);
	if (dosHeader == nullptr) return 0;
	auto* const ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(scanBytes + dosHeader->e_lfanew);
	const auto sizeOfCode = ntHeaders->OptionalHeader.SizeOfImage;

	const auto pattern = pattern_to_byte(signature);
	const auto end = scanBytes + sizeOfCode;

	auto it = std::search(
		scanBytes, end,
		pattern.cbegin(), pattern.cend(),
		[](auto byte, auto opt) {
			return !opt.has_value() || *opt == byte;
		});

	auto ret = it != end ? (unsigned long long)it : 0u;
	if (!ret)
		return 0;

	return ret;
}

auto Mem::findSigRelay(uintptr_t ptr, const char* signature, size_t rang) -> uintptr_t
{
	const char* pattern = signature;
	for (;;) {
		if (rang <= 0) return 0;
		pattern = signature;
		uintptr_t startPtr = ptr;
		for (;;) {
			if (*pattern == ' ') {
				pattern++;
			}
			if (*pattern == '\0') {
				return ptr;
			}
			if (*pattern == '\?') {
				pattern++;
				startPtr++;
				continue;
			}
			if (*(BYTE*)startPtr == GET_BYTE(pattern)) {
				pattern += 2;
				startPtr++;
				continue;
			}
			break;
		}
		ptr++;
		rang--;
	}
}

