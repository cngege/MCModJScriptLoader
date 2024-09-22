#include "mem.h"

#include <Windows.h>	//GetModuleHandleA
#include <vector>
#include <optional>
#include <algorithm>	//std::search
#include <unordered_map>
#include <string>

#define INRANGE(x,a,b) (x >= a && x <= b)
#define GET_BYTE( x )  (GET_BITS(x[0]) << 4 | GET_BITS(x[1]))
#define GET_BITS( x )  (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))

static std::unordered_map<std::string, uintptr_t> signmap{};

auto Mem::findSig(const char* signature, const char* modulename) -> uintptr_t
{
	auto sign_it = signmap.find(std::string(signature));
	if(sign_it != signmap.end()) {
		return sign_it->second;
	}

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

	signmap[std::string(signature)] = ret;
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

auto Mem::funcFromSigOffset(uintptr_t sig, int offset) -> uintptr_t {
	auto jmpval = *reinterpret_cast<int*>(sig + offset);
	return sig + offset + 4 + jmpval;
}

auto Mem::getBase(const char* module) ->uintptr_t {
	return (uintptr_t)GetModuleHandleA(module);
}