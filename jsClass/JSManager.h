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

#pragma region JSManager

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
    // 加载首个远程模块
    auto loadModuleFromHttp(const std::string&) -> JSModuleDef*;
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

    auto initJSManager()->void;
    auto disableJSManager() -> void;

private:
    JSContext* m_ctx = nullptr;
};

#pragma endregion
// endregion JSManager

#pragma region JSTool


class JSTool {
public:
    class JSClassRegister {
        JSClassID* m_id = nullptr;
        JSClassDef* m_class_def = nullptr;
        const char* m_class_name;
        JSValue protoInstance = JS_NULL;/*protoInstance*/
        JSValue ctroInstance = JS_NULL;/*ctroInstance*/
    public:
        JSClassRegister(JSClassID* id, JSClassDef* class_def, const char* class_name = nullptr){
            auto ctx = JSManager::getInstance()->getctx();
            auto rt = JS_GetRuntime(ctx);
            m_id = id;
            JS_NewClassID(m_id);
            m_class_def = class_def;
            JS_NewClass(rt, *m_id, m_class_def);
            if(class_name == nullptr)
                m_class_name = class_def->class_name;
            else
                m_class_name = class_name;
            // 创建一个对象用户存放所有属性函数
            protoInstance = JS_NewObject(ctx);
        }

        /**
         * @brief 设置绑定此类的构造函数
         * @param fun C++函数
         * @param length 期望的参数个数：0
         * @param magic 预制的magic:0
         * @return 当前注册类实例
         */
        JSClassRegister& setConstructor(JSCFunction* fun,int length = 0, int magic = 0) {
            auto ctx = JSManager::getInstance()->getctx();
            ctroInstance = JS_NewCFunction2(ctx, fun, m_class_name, length, JS_CFUNC_constructor, magic);
            JS_SetConstructor(ctx, ctroInstance, protoInstance); /*将此构造函数绑定到类中*/
            JS_SetClassProto(ctx, *m_id, protoInstance); /*绑定此类的id*/
            return *this;
        }
        /**
         * @brief 注册绑定一个C++类到JS属性
         * @param fun 绑定的C++函数
         * @param name 函数在JS中可获取的名称
         * @param length 期望的参数个数：0
         * @return 
         */
        JSClassRegister& setPropFunc(JSCFunction* fun, std::string name, int length = 0) {
            JSTool::setPropFunc(protoInstance, fun, name.c_str(), length);
            return *this;
        }
        /**
         * @brief 将此类作为objJS对象的属性类
         * @param obj 如果obj是默认值JS_NULL,则绑定到公共对象上
         */
        void build(JSValue obj = JS_NULL) {
            auto ctx = JSManager::getInstance()->getctx();
            if(JS_IsNull(obj)) {
                JSValue m_glboalObj = JS_GetGlobalObject(ctx);
                JS_SetPropertyStr(ctx, m_glboalObj, m_class_name, ctroInstance);
                JS_FreeValue(ctx, m_glboalObj);
            }else{
                JS_SetPropertyStr(ctx, obj, m_class_name, ctroInstance);
            }

            JS_FreeValue(ctx, protoInstance);
            JS_FreeValue(ctx, ctroInstance);
        }
        /**
         * @brief 将此类注册实例的接口返回，可供模块调用
         * @return 
         */
        JSValue buildToModule() const {
            auto ctx = JSManager::getInstance()->getctx();
            JS_FreeValue(ctx, protoInstance);
            return ctroInstance;
        }

        ~JSClassRegister() {}
    };


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
    static std::optional<std::array<float, 3>> getPropRGB(JSValue);

    static std::optional<std::array<float, 4>> getPropXYZW(JSValue);
    static std::optional<std::array<float, 4>> getPropRGBA(JSValue);

    /**
     * @brief 给一个JS值设置对象属性
     * @param  
     * @param  
     */
    static bool setPropXY(JSValue, std::array<float, 2>);
    /**
     * @brief 给一个JS值设置对象属性
     * @param
     * @param
     */
    static bool setPropXYZ(JSValue, std::array<float, 3>);
    /**
     * @brief 给一个JS值设置对象属性
     * @param
     * @param
     */
    static bool setPropXYZW(JSValue, std::array<float, 4>);
    /**
     * @brief 给一个JS值设置对象属性
     * @param
     * @param
     */
    static bool setPropRGB(JSValue, std::array<float, 3>);
    /**
     * @brief 给一个JS值设置对象属性
     * @param
     * @param
     */
    static bool setPropRGBA(JSValue, std::array<float, 4>);

    /**
     * @brief 给属性添加一个字符串对象对象为函数
     * @param  父对象
     * @param fun 绑定的C++函数
     * @param str 映射到JS的函数名称
     * @param length 期待的参数数量(默认0)
     * @return 
     */
    static int setPropFunc(JSValue, JSCFunction* fun, std::string str,int length = 0);
    //static int setPropFunc(JSValue, JSCFunction* fun, const char* str, int length = 0, int magic = 0);
    
    /**
     * @brief 专门用于构造函数获取一个支持继承的原型链
     * @param thisObj 构造函数的this对象
     * @param id 当前内置类的id
     * @param inheritance 是否允许被继承(true)
     * @return 此值最终作为返回值return
     */
    static JSValue getConstructorValue(JSValue thisObj, JSClassID id, bool inheritance = true);

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

    static std::optional<std::vector<std::pair<std::string, std::string>>> getObjKV(JSValue jsv);

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

    static bool isFun(JSValue jsv);

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
        Param& Parse(std::optional<T>& ref);

        Param& Parse(bool need = true);

        //template<typename T>
        //Param& ParseCall(std::function<T(T)>& refcall);
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

    /**
     * @brief 为了适配ImGui中的指针引用,采用回调函数的方式改写值
     * @param  某个参数，比如为回调
     * @param call 使用call进行上下文处理
     * @return 错误字符串，enemy 不为空的时候表示有错误
     */
    static std::string ReferenceBoolCall(JSValue, std::function<void(bool*)> call);
    /**
     * @brief 为了适配ImGui中的指针引用,采用回调函数的方式改写值
     * @param  某个参数，比如为回调
     * @param call 使用call进行上下文处理
     * @return 错误字符串，enemy 不为空的时候表示有错误
     */
    static std::string ReferenceIntCall(JSValue, std::function<void(int*)> call);
    /**
     * @brief 为了适配ImGui中的指针引用,采用回调函数的方式改写值
     * @param  某个参数，比如为回调
     * @param call 使用call进行上下文处理
     * @return 错误字符串，enemy 不为空的时候表示有错误
     */
    static std::string ReferenceFloatCall(JSValue, std::function<void(float*)> call);
    /**
     * @brief 为了适配ImGui中的指针引用,采用回调函数的方式改写值
     * @param  某个参数，比如为回调
     * @param call 使用call进行上下文处理
     * @return 错误字符串，enemy 不为空的时候表示有错误
     */
    static std::string ReferenceDoubleCall(JSValue, std::function<void(double*)> call);
    static std::string ReferenceStringCall(JSValue, size_t, std::function<void(char*)> call);

    static std::string ReferenceVec2PropCall(JSValue, std::function<void(float*)> call);
    static std::string ReferenceVec3PropCall(JSValue, std::function<void(float*)> call);
    static std::string ReferenceVec4PropCall(JSValue, std::function<void(float*)> call);

    static int iMin(size_t v1, int v2);
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
    auto ctx = JSManager::getInstance()->getctx();
    using DecayT = std::decay_t<T>;

    if(m_hasErr) return *this;
    if(m_index >= m_argc) { // 不够
        // 不够也要分情况，如果当前值是可选的, 那么没事
        if(!ref.has_value()) {
            m_hasErr = true;
            m_JSErr = std::format("仅有{}个参数是不够的，正在尝试读取第{}个参数 in:{}:{}", m_argc, m_index + 1, __FILE__, __LINE__);
        }
        m_index++;
        return *this;
    }
    // 只要这个参数在C++中有预设值，那就允许js传参为null
    if((JS_IsNull(m_argv[m_index]) || JS_IsUndefined(m_argv[m_index])) && ref) {
        m_index++;
        return *this;
    }

    if constexpr(std::is_same_v<DecayT, bool>) {

        auto v = JS_ToBool(ctx, m_argv[m_index]);
        if(v < 0) {
            m_hasErr = true;
            m_JSErr = std::format("{} 号参数转为{}类型值失败 in:{}:{}", m_index, "int", __FILE__, __LINE__);
        }
        else {
            ref = v;
        }
        m_index++;
        return *this;
    }
    else if constexpr(std::is_same_v<DecayT, int>) {
        
        auto v = toInt(m_argv[m_index]);
        if(!v) {
            m_hasErr = true;
            m_JSErr = std::format("{} 号参数转为{}类型值失败 in:{}:{}", m_index, "int", __FILE__, __LINE__);
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
            m_JSErr = std::format("{} 号参数转为{}类型值失败 in:{}:{}", m_index, "int64", __FILE__, __LINE__);
        }
        else {
            ref = *v;
        }
        m_index++;
        return *this;
    }
    else if constexpr(std::is_same_v<DecayT, size_t>) {
        auto v = toInt64(m_argv[m_index]);
        if(!v) {
            m_hasErr = true;
            m_JSErr = std::format("{} 号参数转为{}类型值失败 in:{}:{}", m_index, "size_t",__FILE__, __LINE__);
        }
        else {
            ref = (size_t)*v;
        }
        m_index++;
        return *this;
    }
    else if constexpr(std::is_same_v<DecayT, float>) {
        auto v = toFloat(m_argv[m_index]);
        if(!v) {
            m_hasErr = true;
            m_JSErr = std::format("{} 号参数转为{}类型值失败 in:{}:{}", m_index, "float", __FILE__, __LINE__);
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
            m_JSErr = std::format("{} 号参数转为{}类型值失败 in:{}:{}", m_index, "double", __FILE__, __LINE__);
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
            m_JSErr = std::format("{} 号参数转为{}类型值失败 in:{}:{}", m_index, "ImVec2", __FILE__, __LINE__);
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
            m_JSErr = std::format("{} 号参数转为{}类型值失败 in:{}:{}", m_index, "string", __FILE__, __LINE__);
        }
        else {
            ref = *v;
        }
        m_index++;
        return *this;
    }
    else if constexpr(std::is_same_v<DecayT, std::vector<std::string>>) {
        try {
            auto v = getArray<std::string>(m_argv[m_index], [=](size_t i, JSValue jsv) {
                auto readjsvstr = JSTool::toString(jsv);
                if(!readjsvstr) {
                    throw std::runtime_error(std::format("读取转化JS数组{}号成员为{}时失败 in:{}:{}",i, "字符串", __FILE__, __LINE__));
                }
                else {
                    return *readjsvstr;
                }
            });
            if(!v) {
                m_hasErr = true;
                m_JSErr = std::format("{} 号参数转为{}类型值失败 in:{}:{}", m_index, "string[]", __FILE__, __LINE__);
            }
            else {
                ref = *v;
            }
        }
        catch(std::exception& ex) {
            m_hasErr = true;
            m_JSErr = std::format("{} 号参数转为{}类型值失败，错误:{}  in:{}:{}", m_index, "string[]", ex.what(), __FILE__, __LINE__);
        }

        m_index++;
        return *this;
    }
    else {
        m_hasErr = true;
        m_JSErr = std::format("{} 号参数转为{}类型值失败 in:{}:{}", m_index, "[未知类型]", __FILE__, __LINE__);
        m_index++;

        static_assert(std::is_same_v<DecayT, void>, "尚未支持的解析参数");
    }

    return *this;
}


#pragma endregion   
// endregion JSTool


