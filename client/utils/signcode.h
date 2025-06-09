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
	 * @brief 即便特征码已经查找成功，依旧检查剩下的sig是否有效
	 */
	bool _checkallSig = false;
	/**
	 * @brief 如果为true并且 _checkallSig 启用，则在检查特征码时判断最终结果于首个有效值是否相差过大
	 */
	bool _judgeDifference = true;
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
	std::string _printTitle = "";
	/**
	 * @brief 成功找到地址的有效特征码
	*/
	std::string validMemcode = "";
	/**
	 * @brief 使用特征码直接找的没有进行偏移处理的地址
	*/
	uintptr_t validPtr = 0;
	/**
	 * @brief 深度查找
	 */
	bool m_deepSearch = false;

public:
	using SignHandle = uintptr_t(__fastcall*)(uintptr_t);

	/**
	 * @brief 特征码查找工具
	 * @param title 标题，用于输出提示信息时区别不同的SignCode
	 * @param printfail 是否在部分特征码无效时警告
	 * @param checkAllSig 是否即使已经查找到结果后依旧检查其他特征码
	 * @param judgeDifference 检查其他特征码时 是否检查结果与最初有效值是否区别太大
	 */
	SignCode(std::string title, bool printfail = true, bool checkAllSig = false, bool judgeDifference = true) :
		_printTitle(title), _printfail(printfail), _checkallSig(checkAllSig), _judgeDifference(judgeDifference)
	{};

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
	std::string ValidSign() const;
	/**
	 * @brief 获取使用特征码直接找的没有进行偏移处理的地址
	 * @return
	*/
	uintptr_t ValidPtr() const;

	/**
	 * @brief 设置深度,而不是找到一个就结束,内存不友好函数
	 * @param enable 
	 */
	void setDeepSearch(bool enable = false) { m_deepSearch = enable; };

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
