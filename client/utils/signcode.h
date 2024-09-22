#pragma once
#ifndef CLIENT_UTILS_SIGNCODE_H
#define CLIENT_UTILS_SIGNCODE_H
#include <string>
#include <Shlobj.h>
#include <functional>

class SignCode {
	/**
	 * @brief 是否最终成功获取到地址
	*/
	bool success = false;
	/**
	 * @brief 是否打印错误信息
	*/
	bool _printfail = true;
	/**
	 * @brief 用于返回的找到的特征码地址
	*/
	uintptr_t v = 0;
	/**
	 * @brief 寻找次数
	*/
	int findCount = 0;
	/**
	 * @brief 关于打印错误信息时的标题
	*/
	const char* _printTitle = "";
	/**
	 * @brief 成功找到地址的有效特征码
	*/
	const char* validMemcode = "";
	/**
	 * @brief 使用特征码直接找的没有进行偏移处理的地址
	*/
	uintptr_t validPtr = 0;

public:
	using SignHandle = uintptr_t(__fastcall*)(uintptr_t);

	//SignCode() {};
	SignCode(const char* title, bool printfail = true) :_printTitle(title), _printfail(printfail) {};

	operator bool() const;

	/**
	 * @brief 获取最终有效地址
	 * @return
	*/
	uintptr_t operator *() const;

	void operator <<(const char* sign);

	void operator <<(std::string sign);

	/**
	 * @brief 获取最终有效地址 或者直接使用 *(obj) 获取
	 * @return
	*/
	uintptr_t get() const;

	/**
	 * @brief 获取能成功定位到有效地址的特征码
	 * @return
	*/
	const char* ValidSign() const;
	/**
	 * @brief 获取使用特征码直接找的没有进行偏移处理的地址
	 * @return
	*/
	uintptr_t ValidPtr() const;

	/**
	 * @brief 传入直接定位地址的特征码
	 * @param sign
	 * @param handle 获取成功后 二次处理, 返回值为最终结果
	*/
	void AddSign(const char* sign, std::function<uintptr_t(uintptr_t)> handle = nullptr);

	/**
	 * @brief 传入调用处的特征码
	 * @param sign
	 * @param offset
	 * @param handle 获取成功后 二次处理, 返回值为最终结果
	*/
	void AddSignCall(const char* sign, int offset = 1, std::function<uintptr_t(uintptr_t)> handle = nullptr);
};
#endif
