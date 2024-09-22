#include "signcode.h"
#include "spdlog/spdlog.h"
#include "../mem/mem.h"


SignCode::operator bool() const {
	if(!success && _printfail) {
		spdlog::error("SignCode Error: {} 没能从特征码定位到地址", this->_printTitle);
	}
	return success;
}

uintptr_t SignCode::operator *() const {
	return v;
}

void SignCode::operator <<(const char* sign) {
	AddSign(sign);
}

void SignCode::operator <<(std::string sign) {
	AddSign(sign.c_str());
}

uintptr_t SignCode::get() const {
	return v;
}

const char* SignCode::ValidSign() const {
	return validMemcode;
}

uintptr_t SignCode::ValidPtr() const {
	return validPtr;
}

void SignCode::AddSign(const char* sign, std::function<uintptr_t(uintptr_t)> handle) {
	findCount++;
	if(success) return;
	v = Mem::findSig(sign);
	if(!v) {
		spdlog::error("SignCode Warn: {} 特征码查找失败({})", this->_printTitle, this->findCount);
	}
	else {
		success = true;
		validMemcode = sign;
		validPtr = v;
		if(handle != nullptr) {
			v = handle(v);
			if(v == 0) {
				success = false;
				return;
			}
		}
	}
}

void SignCode::AddSignCall(const char* sign, int offset, std::function<uintptr_t(uintptr_t)> handle) {
	findCount++;
	if(success) return;
	auto _v = Mem::findSig(sign);
	if(!_v) {
		spdlog::error("SignCode Warn: {} 特征码查找失败({})", this->_printTitle, this->findCount);
	}
	else {
		success = true;
		validMemcode = sign;
		validPtr = _v;
		v = Mem::funcFromSigOffset(_v, offset);
		if(handle != nullptr) {
			v = handle(v);
			if(v == 0) {
				success = false;
				return;
			}
		}
	}
}

