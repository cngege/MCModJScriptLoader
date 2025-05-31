// MCModJScriptLoader.cpp: 定义应用程序的入口点。
//

#include <fstream>
#include <iostream>

#include "hook/HookManager.h"
#include "client/ModManager.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/callback_sink.h"

#include "imgui_kiero/kiero.h"
#include "hook/HookImgui.h"

#include "client/mem/mem.h"
#include "client/utils/signcode.h"

#include "client/utils/assets_net.h"

#include "imgui/imgui_uwp_wndProc.h"

#include "quickjs/quickjs-libc.h"

#include "jsClass/JSManager.h"
#include "jsClass/spdlog/spdlogClass.h"
#include "jsClass/otherProperties/otherJSClass.h"
#include "jsClass/hook/hookClass.h"
#include "jsClass/nativePoint/nativePointClass.h"

// TODO:使用 '#' 符号开头表示github 包
// TODO:...
static JSModuleDef* js_module_loader_local(JSContext* ctx, const char* module_name, void* opaque) {
    std::string module = module_name;
    if(std::string(module_name).starts_with("http://") || std::string(module_name).starts_with("https://")) {
        std::string url = module_name;
        return JSManager::getInstance()->loadModuleFromHttp(url);
    }
    else {
        if(module.ends_with(".js") || module.ends_with(".mjs")) {                                                // 如果是以JS或者TS结尾
            return JSManager::getInstance()->loadModuleFromFile((ModManager::getInstance()->getPath("script") / module_name).string());
        }
        else if(fs::exists(ModManager::getInstance()->getPath("script") / (module + ".js"))) {                                                                                     // 如果补全.js后文件存在
            return JSManager::getInstance()->loadModuleFromFile((ModManager::getInstance()->getPath("script") / module_name).string() + ".js");
        }
        else if(fs::exists(ModManager::getInstance()->getPath("script") / (module + ".mjs"))) {                                                                                    // 如果补全 ".mjs" 后文件存在
            return JSManager::getInstance()->loadModuleFromFile((ModManager::getInstance()->getPath("script") / module_name).string() + ".mjs");
        }
        else {
            return js_module_loader(ctx, module_name, opaque);
        }
    }
}

//BedrockLogOut
uintptr_t gamelogger_ori = 0;
HookInstance* gamelogger_info = nullptr;
static void BedrockLogOutHook(UINT type, const char* fmt, ...) {
    try {
        va_list va;
        va_start(va, fmt);
        char tn[0x1000];
        memset(tn, '\0', 0x1000);
        vsnprintf(tn, sizeof(tn) - 1, fmt, va);
        if(tn[strlen(tn) - 1] == '\n') {
            tn[strlen(tn) - 1] = '\0';
        }
        switch(type) {
        case 1:
            spdlog::debug(tn);
            break;
        case 2:
            spdlog::info(tn);
            break;
        case 8:
            spdlog::error(tn);
            break;
        case 4:
        default:
            spdlog::warn(tn);
            break;
        }
        reinterpret_cast<void(__fastcall*)(UINT, va_list)>(gamelogger_info->origin)(type, va);
        va_end(va);
    }
    catch(...) {
        spdlog::error("游戏内部日志输出错误...");
    }
}


static JSRuntime* rt = nullptr;
static JSContext* ctx = nullptr;
auto stop(HMODULE) -> void;
auto test() -> void;

static auto start(HMODULE hModule) -> void {
    char* localAppData = nullptr;
    size_t localsize = 0;
    _dupenv_s(&localAppData, &localsize, "LOCALAPPDATA");
    if(localAppData == nullptr) {
        throw std::runtime_error("取环境变量 LOCALAPPDATA 失败");
        return;
    }
    //const char* local = getenv("LOCALAPPDATA");//C:\Users\CNGEGE\AppData\Local\Packages\microsoft.minecraftuwp_8wekyb3d8bbwe\AC
    // 中文字体：https://ghproxy.cc/https://github.com/cngege/MCModJScriptLoader/releases/download/0.0.1/JNMYT.ttf
    fs::path moduleDir = std::string(localAppData) + "\\..\\RoamingState\\JSRunner";
    ModManager::getInstance()->setModulePath(moduleDir);

    ModManager::getInstance()->pathCreate("");
    ModManager::getInstance()->pathCreate("script");
    ModManager::getInstance()->pathCreate("script/module");
    ModManager::getInstance()->pathCreate("config");
    ModManager::getInstance()->pathCreate("Assets");
    ModManager::getInstance()->pathCreate("Assets/Fonts");

    if (fs::exists(moduleDir / "app.log")) {
        fs::remove(moduleDir / "app.log");
    }
    ModManager::getInstance()->setImConfigPath("config/imgui.ini");
    ModManager::getInstance()->setImLogPath("config/imgui_log.ini");

    ModManager::getInstance()->setOtherPath("ModConfig", "config/config.json");
    if(!fs::exists(ModManager::getInstance()->getOtherPath("ModConfig"))) {
        std::fstream config(ModManager::getInstance()->getOtherPath("ModConfig").string(), std::ios::out);
        if(config.is_open()) {
            config << "{}" << std::endl;
            config.close();
        }
    }

    auto file_logger = spdlog::basic_logger_mt("MCJSRunTime", ModManager::getInstance()->getPath("app.log").string());
    file_logger->sinks().push_back(std::make_shared<spdlog::sinks::callback_sink_mt>([](const spdlog::details::log_msg& msg) {
        switch(msg.level) {
        case spdlog::level::info:
            GetImguiConsole()->AddLog("[info] %s", std::string(msg.payload.begin(), msg.payload.end()).c_str());
            break;
        case spdlog::level::warn:
            GetImguiConsole()->AddLog("[warn] %s", std::string(msg.payload.begin(), msg.payload.end()).c_str());
            break;
        case spdlog::level::err:
            GetImguiConsole()->AddLog("[error] %s", std::string(msg.payload.begin(), msg.payload.end()).c_str());
            GetImguiConsole()->MainOpen = true;
            break;
        case spdlog::level::debug:
            GetImguiConsole()->AddLog("[debug] %s", std::string(msg.payload.begin(), msg.payload.end()).c_str());
            break;
        case spdlog::level::trace:
            GetImguiConsole()->AddLog("[trace] %s", std::string(msg.payload.begin(), msg.payload.end()).c_str());
            break;
        case spdlog::level::critical:
            GetImguiConsole()->AddLog("[critical] %s", std::string(msg.payload.begin(), msg.payload.end()).c_str());
            break;
        default:
            break;
        }
    }));
    spdlog::set_default_logger(file_logger);

    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);  // 日志保存等级
    spdlog::info("日志部分完工撒花..");
    
    ImguiHooks::InitImgui();

    // httplib 下载字体文件
    http::downFont_JNMYT(ModManager::getInstance()->getPath("Assets/Fonts"));

    // BedrockLogOut 函数定位 内部有字符串 "!!! ERROR: Unable to format log output message !!!"
    {
        SignCode sign("BedrockLogOut");
        sign << "48 89 54 24 ? 4C 89 44 24 ? 4C 89 4C 24 ? 55 53 56 57 41 54 41 56 41 57 48";
        sign.AddSignCall("47 ? ? 48 8D 15 ? ? ? ? 41 8B ? E8", 14);
        if(sign) {
            gamelogger_info = HookManager::addHook(*sign, (void*)&BedrockLogOutHook);
            gamelogger_info->hook();
        }
        else {
            spdlog::warn("gamelogger Hook fail. in fun:{}", __FUNCTION__);
        }
    }

    //JS Runner
    rt = JS_NewRuntime();
    ctx = JS_NewContext(rt);
    js_std_init_handlers(rt);
    JSManager::getInstance()->setctx(ctx);
    JSManager::getInstance()->initJSManager();

    // 开启BigNumber
    //JS_AddIntrinsicBigFloat(ctx);
    //JS_AddIntrinsicBigDecimal(ctx);
    //JS_AddIntrinsicOperators(ctx);
    //JS_EnableBignumExt(ctx, true);

    JS_SetModuleLoaderFunc(rt, nullptr, js_module_loader_local, nullptr);
    
    JSManager::getInstance()->loadNativeModule();

    spdlogClass::Reg();
    hookClass::Reg();
    nativePointClass::Reg();
    otherJSClass::Reg();

    JSManager::getInstance()->loadJSFromFoder();

    JSManager::getInstance()->registerImGuiMouseHandle();       //注册鼠标按键相关事件
    JSManager::getInstance()->runstdLoop();                     // 耗时操作， 跑完JS队列， 使 setTimeout工作
    test();
    ModManager::getInstance()->loopback();                      // 循环等待卸载
    stop(hModule);
    Sleep(100);
    ::FreeLibraryAndExitThread(hModule, 0);                     //只能退出 CreateThread 创建的线程
}

static auto stop(HMODULE hModule)->void {
    try {
        //卸载Hook
        HookManager::disableAllHook();
        Sleep(10);
        ImguiHooks::CloseImGui();


        // 释放JS接口申请的资源
        spdlogClass::Dispose();
        hookClass::Dispose();
        nativePointClass::Dispose();
        otherJSClass::Dispose();

        // 释放模块资源
        JSManager::getInstance()->freeNativeModule(rt);
        JSManager::getInstance()->disableJSManager();

        ModManager::getInstance()->disableMod((uintptr_t)hModule);

        // JS释放
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);

        // spdlog shutdown
        spdlog::shutdown();
    }
    catch(std::exception&) {}
}

// Dll入口函数
auto APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) -> BOOL {
    DisableThreadLibraryCalls(hModule);//应用程序及其DLL的线程创建与销毁不再对此DLL进行通知
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)start, hModule, NULL, nullptr);
    }
    //FreeLibrary()
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        ModManager::getInstance()->stopSign();
    }
    
    return TRUE;
}

void test() {
    //auto core = *(CoreWindow*)ModManager::getCoreWindow();
    //spdlog::warn("Text CoreWindow Ptr: {} v: {}", (void*)core, *(void**)core);
    //auto keyd = TypedEventHandler<CoreWindow, IKeyEventArgs>(&keydown);
    //core->KeyDown(TypedEventHandler<CoreWindow, KeyEventArgs>(&keydown));
    //auto pos = core.PointerPosition();
    //spdlog::warn("X:{}, Y:{}", pos.X, pos.Y);
}


/*
* https://cloud.tencent.com/developer/article/1879884
* https://blog.csdn.net/qq_36291960/article/details/109048424
JS_ThrowSyntaxError  解析/编译期间发现的语法错误
JS_ThrowTypeError 对错误的数据类型执行操作时会发生TypeError
JS_ThrowReferenceError   引用错误 引用一个不存在的变量时发生的错误 如果我们调用的是一个已经存在的变量的一个不存在的属性，则不会抛出 ReferenceError，因为变量本身已经在存储中了，调用它不存在的属性只会是未定义状态，也就是 undefined:
JS_ThrowRangeError  边界错误 表示超出有效范围时发生的异常，主要的有以下几种情况：数组长度为负数或超长 数字类型的方法参数超出预定义范围 函数堆栈调用超过最大值
JS_ThrowOutOfMemory 内存不足相关
JS_ThrowInternalError  该错误在JS引擎内部发生，特别是当它有太多数据要处理并且堆栈增长超过其关键限制时。当JS引擎被太多的递归，太多的切换情况等淹没时，就会发生这种情况

*/

/* 如何获取CoreWindow 单例
* [*]  命名空间：#include <winrt/windows.ui.core.h> | using namespace winrt::Windows::UI::Core; | CoreWindow
* 原理 在鼠标按键事件回调中 调用 CoreWindow::GetForCurrentThread()， 获取 CoreWindow 对象单例
* - 使用&取存储的引用地址，得到后在使用*取值转为地址，则得到一个地址即 CoreWindow 对象的地址
* - 在CE中搜索这个地址，则在下面能得到 Minecraft.Windows.exe + XXXX 的静态地址, 则找到了Minecraft for UWP 的 CoreWindow 对象
*
*/


