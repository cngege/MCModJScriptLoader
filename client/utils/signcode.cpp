﻿#include "signcode.h"
#include "spdlog/spdlog.h"
#include "../mem/mem.h"
#include <cmath>

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

std::string SignCode::ValidSign() const {
    return validMemcode;
}

uintptr_t SignCode::ValidPtr() const {
    return validPtr;
}

void SignCode::AddSign(const char* sign, std::function<uintptr_t(uintptr_t)> handle) {
    findCount++;
    if(success && _checkallSig == false) return;
    if(m_deepSearch) {
        if(handle == 0) {
            spdlog::warn("此条查找跳过，开启深度查找必须启用回调");
            return;
        }
        auto _vs = Mem::deepSearchSig(sign);
        if(_vs.size() == 0) {
            spdlog::warn("SignCode Warn: {} 特征码查找失败({})", this->_printTitle, this->findCount);
        }
        else {
            if(!success) {
                for(auto& item : _vs) {
                    auto _v = handle(item);
                    if(_v != 0) {
                        v = _v;
                        validPtr = v;
                        validMemcode = sign;
                        success = true;
                        break;
                    }
                }
                if(!success) {
                    spdlog::warn("SignCode Warn: {} 特征码查找成功({}) 但Call检查失败", this->_printTitle, this->findCount);
                }
            }
            else {
                for(size_t i = 0; i < _vs.size(); i++) {
                    auto _v = handle(_vs[i]);
                    if(_v != 0) {
                        break;
                    }
                    else if(i == _vs.size() - 1) {
                        spdlog::warn("SignCode Warn: {} 特征码查找成功({}) 但Call检查失败", this->_printTitle, this->findCount);
                    }
                }
            }
        }
    }
    else {
        auto _v = Mem::findSig(sign);
        if(!_v) {
            spdlog::warn("SignCode Warn: {} 特征码查找失败({})", this->_printTitle, this->findCount);
        }
        else {
            if(!success) {
                success = true;
                validMemcode = sign;
                v = _v;
                validPtr = v;

                if(handle != nullptr) {
                    v = handle(_v);
                    if(v == 0) {
                        success = false;
                        spdlog::warn("SignCode Warn: {} 特征码查找成功({}) 但Call检查失败", this->_printTitle, this->findCount);
                        return;
                    }
                }
            }
            else {
                if(handle != nullptr) {
                    _v = handle(_v);
                    if(_v == 0) {
                        spdlog::warn("SignCode Warn: {} 特征码查找成功({}) 但Call检查失败", this->_printTitle, this->findCount);
                        return;
                    }
                    else {
                        if(_judgeDifference) {                    // 如果最终有效地址与首个有效地址相差超过 0x10000 字节, 则告警
                            if(std::abs((long long)(v - _v)) > 0x10000) {
                                spdlog::warn("SignCode Warn: {} 特征码与Call均查找成功({}) 但此最终值与首个有效值相差甚大(0x10000)", this->_printTitle, this->findCount);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

}

void SignCode::AddSignCall(const char* sign, int offset, std::function<uintptr_t(uintptr_t)> handle) {
    findCount++;
    if(success && _checkallSig == false) return;
    if(m_deepSearch) {
        if(handle == 0) {
            spdlog::warn("此条查找跳过，开启深度查找必须启用回调");
            return;
        }
        auto _vs = Mem::deepSearchSig(sign);
        if(_vs.size() == 0) {
            spdlog::warn("SignCode Warn: {} 特征码查找失败({})", this->_printTitle, this->findCount);
        }
        else {
            if(!success) {
                for(auto& item : _vs) {
                    auto _v = Mem::funcFromSigOffset(item, offset);
                    _v = handle(_v);
                    if(_v != 0) {
                        v = _v;
                        validPtr = v;
                        validMemcode = sign;
                        success = true;
                        break;
                    }
                }
                if(!success) {
                    spdlog::warn("SignCode Warn: {} 特征码查找成功({}) 但Call检查失败", this->_printTitle, this->findCount);
                }
            }
            else {
                for(size_t i = 0; i < _vs.size(); i++) {
                    auto _v = Mem::funcFromSigOffset(_vs[i], offset);
                    _v = handle(_v);
                    if(_v != 0) {
                        break;
                    }
                    else if(i == _vs.size() - 1) {
                        spdlog::warn("SignCode Warn: {} 特征码查找成功({}) 但Call检查失败", this->_printTitle, this->findCount);
                    }
                }
            }
        }
    }
    else {
        auto _v = Mem::findSig(sign);
        if(!_v) {
            spdlog::warn("SignCode Warn: {} 特征码查找失败({})", this->_printTitle, this->findCount);
        }
        else {
            if(!success) {
                success = true;
                validMemcode = sign;
                validPtr = _v;
                v = Mem::funcFromSigOffset(_v, offset);
                if(handle != nullptr) {
                    v = handle(v);
                    if(v == 0) {
                        success = false;
                        spdlog::warn("SignCode Warn: {} 特征码查找成功({}) 但Call检查失败", this->_printTitle, this->findCount);
                        return;
                    }
                }
            }
            else {
                _v = Mem::funcFromSigOffset(_v, offset);
                if(handle != nullptr) {
                    _v = handle(_v);
                    if(_v == 0) {
                        success = false;
                        spdlog::warn("SignCode Warn: {} 特征码查找成功({}) 但Call检查失败", this->_printTitle, this->findCount);
                        return;
                    }
                    else {
                        if(_judgeDifference) {
                            // 如果最终有效地址与首个有效地址相差超过 0x10000 字节, 则告警
                            if(std::abs((long long)(v - _v)) > 0x10000) {
                                spdlog::warn("SignCode Warn: {} 特征码与Call均查找成功({}) 但此最终值与首个有效值相差甚大(0x10000)", this->_printTitle, this->findCount);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
}

