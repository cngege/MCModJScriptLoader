#pragma once
#include "quickjs/quickjs-libc.h"
#include <optional>
#include <functional>
#include <string>
#include <array>
#include <imgui/imgui.h>
#include <format>


#ifndef JS_CFUNC_DEF2
#define JS_CFUNC_DEF2(name, length, func1)                                                                                              \
    {                                                                                                                                  \
        name, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, 0, {.func = {length, JS_CFUNC_generic, {.generic = func1}} }		\
    }
#endif // !JS_CFUNC_DEF2


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

    auto loadConfig() -> void;
    auto saveConfig() -> void;

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
    /**
     * @brief 尝试从JS值中读取字符串
     * @param  
     * @return 
     */
    static std::optional<std::string> toString(JSValue);
    /**
     * @brief 尝试从JS值中读取 int32
     * @param
     * @return
     */
    static std::optional<int32_t> toInt(JSValue);
    /**
     * @brief 尝试从JS值中读取int64
     * @param
     * @return
     */
    static std::optional<int64_t> toInt64(JSValue);
    /**
     * @brief 尝试从JS值中读取float
     * @param
     * @return
     */
    static std::optional<float> toFloat(JSValue);
    /**
     * @brief 尝试从JS值中读取Double
     * @param
     * @return
     */
    static std::optional<double> toDouble(JSValue);
    /**
     * @brief 尝试从JS值中读取 带xy属性的值 并转为数组
     * @param
     * @return
     */
    static std::optional<std::array<float, 2>> getPropXY(JSValue);
    /**
     * @brief 尝试从JS值中读取 带xyz属性的值 并转为数组
     * @param
     * @return
     */
    static std::optional<std::array<float, 3>> getPropXYZ(JSValue);
    /**
     * @brief 尝试从JS值中以数组的方式读取, 并尝试转为C++数组
     * @param
     * @return
     */
    static std::optional<std::vector<JSValue>> toArray(JSValue);
    /**
     * @brief 尝试读取JS数组
     * @tparam T 
     * @param  
     * @param  并允许用户手动将JS数组值转为匹配的任意类型
     * @return 
     */
    template<typename T>
    static std::optional<std::vector<T>> getArray(JSValue, std::function<T(size_t, JSValue)>);

    /**
     * @brief 从C字符串转为JS值
     * @param  
     * @return 
     */
    static JSValue fromString(const char*);
    /**
     * @brief 从C++字符串转为JS值
     * @param  
     * @return 
     */
    static JSValue fromString(std::string);

    /**
     * @brief 进行JS二进制函数参数解析的中间构建类
     */
    class Param {
    public:
        Param(int argc, JSValue* argv);

        /**
         * @brief 逐次解析参数
         * @tparam T 
         * @param ref 参数传递,修改后传回
         * @return 当前中间类,以支持继续解析或者Build
         */
        template<typename T>
        Param& Parse(std::optional<T>& ref/*, bool isOptional = true*/);
        /**
         * @brief 结束解析，返回最终结果
         * @return 字符串, 非空时表示存在错误
         */
        const std::string Build() const;
    private:
        /**
         * @brief 是否存在错误/异常
         */
        bool m_hasErr = false;
        /**
         * @brief 错误内容
         */
        std::string m_JSErr = {};
        /**
         * @brief 记录总共参数个数
         */
        int m_argc = 0;
        /**
         * @brief 当前解析参数的下标,递增
         */
        int m_index = 0;
        /**
         * @brief 记录JS参数
         */
        JSValue* m_argv = nullptr;
    };

    /**
     * @brief 创建参数解析器
     * @param argc 照搬JS二进制函数的参数个数
     * @param argv 照搬JS二进制函数的参数数组指针
     * @return 
     */
    static JSTool::Param createParseParameter(int argc, JSValue* argv);

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

template<typename T>
std::optional<std::vector<T>> JSTool::getArray(JSValue v, std::function<T(size_t, JSValue)> call) {
    auto ctx = JSManager::getInstance()->getctx();
    std::optional<std::vector<T>> ret{};
    if(!JS_IsArray(ctx, v)) {
        return ret;
    }
    std::vector<T> retv{};
    JSValue lengthVal = JS_GetPropertyStr(ctx, v, "length");
    uint64_t len = 0;
    JS_ToIndex(ctx, &len, lengthVal);
    JS_FreeValue(ctx, lengthVal);
    for(int i = 0; i < len; ++i) {
        JSValue item = JS_GetPropertyUint32(ctx, v, i);
        retv.push_back(call(i, item));
        JS_FreeValue(ctx, item);
    }
    ret = retv;
    return ret;
}


template<typename T>
inline JSTool::Param& JSTool::Param::Parse(std::optional<T>& ref) {
    //auto ctx = JSManager::getInstance()->getctx();
    using DecayT = std::decay_t<T>;

    if(m_hasErr) return *this;
    if(m_index >= m_argc) { // 不够
        // 不够也要分情况，如果当前值是可选的, 那么没事
        if(!ref.has_value()) {
            m_hasErr = true;
            m_JSErr = std::format("仅有{}个参数是不够的，正在尝试读取第{}个参数", m_argc, m_index + 1);
        }
        m_index++;
        return *this;
    }

    if constexpr(std::is_same_v<DecayT, int>) {
        
        auto v = toInt(m_argv[m_index]);
        if(!v) {
            m_hasErr = true;
            m_JSErr = std::format("{} 号参数转为{}类型值失败", m_index, "int");
        }
        else {
            ref = *v;
        }
        m_index++;
        return *this;
    }
    else if constexpr(std::is_same_v<DecayT, int64_t>) {
        auto v = toInt64(m_argv[m_index]);
        if(!v) {
            m_hasErr = true;
            m_JSErr = std::format("{} 号参数转为{}类型值失败", m_index, "int64");
        }
        else {
            ref = *v;
        }
        m_index++;
        return *this;
    }
    else if constexpr(std::is_same_v<DecayT, float>) {
        auto v = toFloat(m_argv[m_index]);
        if(!v) {
            m_hasErr = true;
            m_JSErr = std::format("{} 号参数转为{}类型值失败", m_index, "float");
        }
        else {
            ref = *v;
        }
        m_index++;
        return *this;
    }
    else if constexpr(std::is_same_v<DecayT, double>) {
        auto v = toDouble(m_argv[m_index]);
        if(!v) {
            m_hasErr = true;
            m_JSErr = std::format("{} 号参数转为{}类型值失败", m_index, "double");
        }
        else {
            ref = *v;
        }
        m_index++;
        return *this;
    }
    else if constexpr(std::is_same_v<DecayT, ImVec2>) {
        auto v = getPropXY(m_argv[m_index]);
        if(!v) {
            m_hasErr = true;
            m_JSErr = std::format("{} 号参数转为{}类型值失败", m_index, "ImVec2");
        }
        else {
            ref = {v->at(0), v->at(1)};
        }
        m_index++;
        return *this;
    }
    else if constexpr(std::is_same_v<DecayT, std::string>) {
        auto v = toString(m_argv[m_index]);
        if(!v) {
            m_hasErr = true;
            m_JSErr = std::format("{} 号参数转为{}类型值失败", m_index, "string");
        }
        else {
            ref = *v;
        }
        m_index++;
        return *this;
    }
    else {
        m_hasErr = true;
        m_JSErr = std::format("{} 号参数转为{}类型值失败", m_index, "[未知类型]");
        m_index++;

        static_assert(std::is_same_v<DecayT, void>, "尚未支持的解析参数");
    }

    return *this;
}
