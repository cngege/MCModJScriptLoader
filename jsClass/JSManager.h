#pragma once
#include "quickjs/quickjs-libc.h"
#include <optional>
#include <string>
#include <array>


class JSManager {
public:
	static JSManager* getInstance();

public:
	auto setctx(JSContext* ctx) -> void;
	auto getctx() -> JSContext* const;

	auto loadNativeModule() -> void;
	auto freeNativeModule(JSRuntime*) -> void;
	// 加载某个JS模块
	auto loadModuleFromFile(const std::string&) -> JSModuleDef*;
	// 从文件夹中加载JS
	auto loadJSFromFoder(const std::string & = "") -> void;
	// 每个 JS脚本加载之前执行的事件
	auto onJsLoadBefore(const std::string, JSValue&) -> void;
	// 每个 JS脚本加载之后执行的事件
	auto onJsLoadAfter(const std::string, JSValue&) -> void;
	// 循环跑完JS队列 - 耗时操作
	auto runstdLoop() -> void;

	auto getErrorStack(JSValue) -> std::string;
	auto getErrorStack() -> std::string;

	auto onImGuiRenderScriptSig() -> void;
	// 存粹的外部ImGui渲染
	auto onImGuiRender() -> void;
	// 使用事件系统注册一个事件，时JS的鼠标Hook能传递到程序中
	auto registerImGuiMouseHandle() -> void;

	auto initJSManager()->void;
	auto disableJSManager() -> void;

private:
	JSContext* m_ctx = nullptr;
};


class JSTool {
public:
	static std::optional<std::string> toString(JSValue);
	static std::optional<int32_t> toInt(JSValue);
	static std::optional<float> toFloat(JSValue);
	static std::optional<double> toDouble(JSValue);
	static std::optional<std::array<float,2>> getPropXY(JSValue);


	/**
	 * @brief 操作引用值工具,将JS传入的对象当作C++引用来操作
	 * @param  将被操作的指针
	 * @param  JSValue
	 * @param  属性名
	 * @return 是否成功,如果false， 则JSValue会被写入错误JS异常
	 */
	static JSValue ReferenceValue(bool*, JSValue, const char* = 0);
	static void ReferenceValue(bool, JSValue, const char* = 0);
	static JSValue ReferenceValue(int*, JSValue, const char* = 0);
	static void ReferenceValue(int, JSValue, const char* = 0);
	static JSValue ReferenceValue(double*, JSValue, const char* = 0);
	static void ReferenceValue(double, JSValue, const char* = 0);
	static JSValue ReferenceValue(float*, JSValue, const char* = 0);
	static void ReferenceValue(float, JSValue, const char* = 0);
};