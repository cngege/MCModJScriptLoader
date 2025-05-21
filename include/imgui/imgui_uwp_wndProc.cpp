#include "imgui_uwp_wndProc.h"

#include <windows.h>
#include <windowsx.h> // GET_X_LPARAM(), GET_Y_LPARAM()
#include "spdlog/spdlog.h"

#include <winrt/windows.ui.core.h>
#include <winrt/windows.ui.input.h>
#include <winrt/base.h>
#include <windows.system.h>
#include <winrt/Windows.Foundation.h>

#include "winrt/windows.applicationmodel.core.h"
#include "winrt/Windows.UI.ViewManagement.h"
//#include "winrt/Windows.UI.Core.h"
#include "winrt/windows.system.h"
using namespace winrt::Windows::UI::Core;

using winrt::Windows::UI::Core::CharacterReceivedEventArgs;
using winrt::Windows::UI::Core::CoreWindow;
using winrt::Windows::UI::Core::KeyEventArgs;
using winrt::Windows::UI::Core::PointerEventArgs;
using winrt::Windows::Foundation::TypedEventHandler;
using winrt::Windows::System::VirtualKey;
using winrt::Windows::UI::Core::CorePhysicalKeyStatus;
using winrt::Windows::UI::Core::CoreVirtualKeyStates;
using namespace winrt::Windows::ApplicationModel::Core;

static CoreWindow win = NULL;

ImGuiMouseSource GetMouseSourceFromMessageExtraInfo()
{
    LPARAM extra_info = ::GetMessageExtraInfo();
    if ((extra_info & 0xFFFFFF80) == 0xFF515700)
        return ImGuiMouseSource_Pen;
    if ((extra_info & 0xFFFFFF80) == 0xFF515780)
        return ImGuiMouseSource_TouchScreen;
    return ImGuiMouseSource_Mouse;
}

void ImGui_ImplUWP_AddKeyEvent(ImGuiKey key, bool down, int native_keycode, int native_scancode = -1)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(key, down);
    io.SetKeyEventNativeData(key, native_keycode, native_scancode); // To support legacy indexing (<1.87 user code)
    IM_UNUSED(native_scancode);
}

static bool IsVkDown(int vk) {
    CoreVirtualKeyStates states = win.GetKeyState(static_cast<VirtualKey>(vk)) & CoreVirtualKeyStates::Down;
    return states == CoreVirtualKeyStates::Down;
}

static void ImGui_ImplUwp_UpdateKeyModifiers() {
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl, IsVkDown(VK_CONTROL));
    io.AddKeyEvent(ImGuiMod_Shift, IsVkDown(VK_SHIFT));
    io.AddKeyEvent(ImGuiMod_Alt, IsVkDown(VK_MENU));
    io.AddKeyEvent(ImGuiMod_Super, IsVkDown(VK_APPS));
}

IMGUI_IMPL_API LRESULT ImGui_UWP_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

#ifdef IMGUIINPUT_USE_WNDPROC

    //logF_Debug("msg: %02x", msg);
    //if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
    //	return 1;
    //}

    switch (msg)
    {
    case DM_POINTERHITTEST:
        break;
    case WM_NCPOINTERDOWN:				// 鼠标按下?
    case WM_POINTERDOWN:
        break;
    case WM_NCPOINTERUP:
    case WM_POINTERUP:
        break;
    case WM_NCPOINTERUPDATE:
    case WM_POINTERUPDATE:	//ok
        break;
    case WM_PARENTNOTIFY:				// 创建销毁子窗口时的消息
        break;
    case WM_POINTERACTIVATE:			// 忽略
        break;
    case WM_POINTERCAPTURECHANGED:		// 可能是鼠标移开游戏窗口
        break;
    case WM_POINTERDEVICECHANGE:		// 忽略 当显示模式缩放时?
    case WM_POINTERDEVICEINRANGE:
    case WM_POINTERDEVICEOUTOFRANGE:
        break;
    case WM_POINTERENTER:				// 鼠标悬停或移动
        break;
    case WM_POINTERLEAVE:				// 移出窗口 ok
        break;
    case WM_POINTERROUTEDAWAY:			// 什么路由到下一进程
    case WM_POINTERROUTEDRELEASED:		// 和跨进程相关
    case WM_POINTERROUTEDTO:
        break;
    case WM_POINTERWHEEL:				// 鼠标滚轮
        break;
    case WM_POINTERHWHEEL:				// 横向滚轮
        break;
    case WM_TOUCHHITTESTING:
        break;
        // 以下是非官方链接中给出的消息
    case WM_MOUSEMOVE:					//0x200
        break;

    case WM_SETCURSOR:	//0x20
    case WM_MOUSEACTIVATE://0x21
    case WM_NCHITTEST:	//0x84 命中测试
    case WM_IME_SETCONTEXT://0x281
    case WM_IME_NOTIFY://0x282
    case WM_SETFOCUS://0x7
    case WM_KILLFOCUS://0x8
    case WM_INPUT://0xFF
    case WM_SYSKEYDOWN://0x104
    case WM_SYSKEYUP://0x105	直接按下F10之类的健
    case WM_SYSCHAR://0x106		ALT+字符健
    case WM_SYSCOMMAND://0x112

        break;
    case 0xC07D://可能是指切换窗口
    case 0xC1D0:
    case 0x349:
        break;
    default:
        logF_Debug("defaultmsg: %02x", msg);
        break;
    }
#endif // IMGUIINPUT_USE_WNDPROC

    switch (msg)
    {
    case WM_SETFOCUS://0x7
    case WM_KILLFOCUS://0x8
        {
            ImGuiIO& io = ImGui::GetIO();
            //io.AddFocusEvent(msg == WM_SETFOCUS);
        }
    default:
        break;
    }

    return 0;
}


#define IM_VK_KEYPAD_ENTER      (VK_RETURN + 256)

static ImGuiKey ImGui_ImplUwp_VirtualKeyToImGuiKey(WPARAM wParam) {
    switch(wParam) {
    case VK_TAB: return ImGuiKey_Tab;
    case VK_LEFT: return ImGuiKey_LeftArrow;
    case VK_RIGHT: return ImGuiKey_RightArrow;
    case VK_UP: return ImGuiKey_UpArrow;
    case VK_DOWN: return ImGuiKey_DownArrow;
    case VK_PRIOR: return ImGuiKey_PageUp;
    case VK_NEXT: return ImGuiKey_PageDown;
    case VK_HOME: return ImGuiKey_Home;
    case VK_END: return ImGuiKey_End;
    case VK_INSERT: return ImGuiKey_Insert;
    case VK_DELETE: return ImGuiKey_Delete;
    case VK_BACK: return ImGuiKey_Backspace;
    case VK_SPACE: return ImGuiKey_Space;
    case VK_RETURN: return ImGuiKey_Enter;
    case VK_ESCAPE: return ImGuiKey_Escape;
    case VK_OEM_7: return ImGuiKey_Apostrophe;
    case VK_OEM_COMMA: return ImGuiKey_Comma;
    case VK_OEM_MINUS: return ImGuiKey_Minus;
    case VK_OEM_PERIOD: return ImGuiKey_Period;
    case VK_OEM_2: return ImGuiKey_Slash;
    case VK_OEM_1: return ImGuiKey_Semicolon;
    case VK_OEM_PLUS: return ImGuiKey_Equal;
    case VK_OEM_4: return ImGuiKey_LeftBracket;
    case VK_OEM_5: return ImGuiKey_Backslash;
    case VK_OEM_6: return ImGuiKey_RightBracket;
    case VK_OEM_3: return ImGuiKey_GraveAccent;
    case VK_CAPITAL: return ImGuiKey_CapsLock;
    case VK_SCROLL: return ImGuiKey_ScrollLock;
    case VK_NUMLOCK: return ImGuiKey_NumLock;
    case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
    case VK_PAUSE: return ImGuiKey_Pause;
    case VK_NUMPAD0: return ImGuiKey_Keypad0;
    case VK_NUMPAD1: return ImGuiKey_Keypad1;
    case VK_NUMPAD2: return ImGuiKey_Keypad2;
    case VK_NUMPAD3: return ImGuiKey_Keypad3;
    case VK_NUMPAD4: return ImGuiKey_Keypad4;
    case VK_NUMPAD5: return ImGuiKey_Keypad5;
    case VK_NUMPAD6: return ImGuiKey_Keypad6;
    case VK_NUMPAD7: return ImGuiKey_Keypad7;
    case VK_NUMPAD8: return ImGuiKey_Keypad8;
    case VK_NUMPAD9: return ImGuiKey_Keypad9;
    case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
    case VK_DIVIDE: return ImGuiKey_KeypadDivide;
    case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
    case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
    case VK_ADD: return ImGuiKey_KeypadAdd;
    case IM_VK_KEYPAD_ENTER: return ImGuiKey_KeypadEnter;
    case VK_LSHIFT: return ImGuiKey_LeftShift;
    case VK_LCONTROL: return ImGuiKey_LeftCtrl;
    case VK_LMENU: return ImGuiKey_LeftAlt;
    case VK_LWIN: return ImGuiKey_LeftSuper;
    case VK_RSHIFT: return ImGuiKey_RightShift;
    case VK_RCONTROL: return ImGuiKey_RightCtrl;
    case VK_RMENU: return ImGuiKey_RightAlt;
    case VK_RWIN: return ImGuiKey_RightSuper;
    case VK_APPS: return ImGuiKey_Menu;
    case '0': return ImGuiKey_0;
    case '1': return ImGuiKey_1;
    case '2': return ImGuiKey_2;
    case '3': return ImGuiKey_3;
    case '4': return ImGuiKey_4;
    case '5': return ImGuiKey_5;
    case '6': return ImGuiKey_6;
    case '7': return ImGuiKey_7;
    case '8': return ImGuiKey_8;
    case '9': return ImGuiKey_9;
    case 'A': return ImGuiKey_A;
    case 'B': return ImGuiKey_B;
    case 'C': return ImGuiKey_C;
    case 'D': return ImGuiKey_D;
    case 'E': return ImGuiKey_E;
    case 'F': return ImGuiKey_F;
    case 'G': return ImGuiKey_G;
    case 'H': return ImGuiKey_H;
    case 'I': return ImGuiKey_I;
    case 'J': return ImGuiKey_J;
    case 'K': return ImGuiKey_K;
    case 'L': return ImGuiKey_L;
    case 'M': return ImGuiKey_M;
    case 'N': return ImGuiKey_N;
    case 'O': return ImGuiKey_O;
    case 'P': return ImGuiKey_P;
    case 'Q': return ImGuiKey_Q;
    case 'R': return ImGuiKey_R;
    case 'S': return ImGuiKey_S;
    case 'T': return ImGuiKey_T;
    case 'U': return ImGuiKey_U;
    case 'V': return ImGuiKey_V;
    case 'W': return ImGuiKey_W;
    case 'X': return ImGuiKey_X;
    case 'Y': return ImGuiKey_Y;
    case 'Z': return ImGuiKey_Z;
    case VK_F1: return ImGuiKey_F1;
    case VK_F2: return ImGuiKey_F2;
    case VK_F3: return ImGuiKey_F3;
    case VK_F4: return ImGuiKey_F4;
    case VK_F5: return ImGuiKey_F5;
    case VK_F6: return ImGuiKey_F6;
    case VK_F7: return ImGuiKey_F7;
    case VK_F8: return ImGuiKey_F8;
    case VK_F9: return ImGuiKey_F9;
    case VK_F10: return ImGuiKey_F10;
    case VK_F11: return ImGuiKey_F11;
    case VK_F12: return ImGuiKey_F12;
    default: return ImGuiKey_None;
    }
}

void keydown(CoreWindow const& sender, KeyEventArgs const& args) {
    if(ImGui::GetCurrentContext() == nullptr)
        return;

    ImGuiIO& io = ImGui::GetIO();

    VirtualKey key;
    CorePhysicalKeyStatus keyStatus;
    key = args.VirtualKey();
    keyStatus = args.KeyStatus();

    const bool is_key_down = true;
    if((int32_t)key < 256) {
        // Submit modifiers
        ImGui_ImplUwp_UpdateKeyModifiers();

        int vk = (int)key;
        if((key == (VirtualKey)VK_RETURN) && keyStatus.IsExtendedKey)
            vk = IM_VK_KEYPAD_ENTER;

        // Submit key event
        const ImGuiKey key = ImGui_ImplUwp_VirtualKeyToImGuiKey(vk);
        const int scancode = keyStatus.ScanCode;
        if(key != ImGuiKey_None)
            ImGui_ImplUWP_AddKeyEvent(key, is_key_down, vk, scancode);

        // Submit individual left/right modifier events
        if(vk == VK_SHIFT) {
            // Important: Shift keys tend to get stuck when pressed together, missing key-up events are corrected in ImGui_ImplUwp_ProcessKeyEventsWorkarounds()
            if(IsVkDown(VK_LSHIFT) == is_key_down) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_LeftShift, is_key_down, VK_LSHIFT, scancode); }
            if(IsVkDown(VK_RSHIFT) == is_key_down) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_RightShift, is_key_down, VK_RSHIFT, scancode); }
        }
        else if(vk == VK_CONTROL) {
            if(IsVkDown(VK_LCONTROL) == is_key_down) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_LeftCtrl, is_key_down, VK_LCONTROL, scancode); }
            if(IsVkDown(VK_RCONTROL) == is_key_down) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_RightCtrl, is_key_down, VK_RCONTROL, scancode); }
        }
        else if(vk == VK_MENU || keyStatus.IsMenuKeyDown) {
            if(IsVkDown(VK_LMENU) == is_key_down) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_LeftAlt, is_key_down, VK_LMENU, scancode); }
            if(IsVkDown(VK_RMENU) == is_key_down) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_RightAlt, is_key_down, VK_RMENU, scancode); }

            if(!IsVkDown(VK_LMENU) && !IsVkDown(VK_RMENU)) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_LeftAlt, is_key_down, VK_LMENU, scancode); }
        }
    }
    if(ImGui::GetIO().WantCaptureKeyboard) {	// 这里应该是指焦点在ImGui上, 可能不在输入框里
        //args.Handled(true);
    }
}

void keyup(CoreWindow const& sender, KeyEventArgs const& args) {
    if(ImGui::GetCurrentContext() == nullptr)
        return;

    ImGuiIO& io = ImGui::GetIO();

    VirtualKey key;
    CorePhysicalKeyStatus keyStatus;
    key = args.VirtualKey();
    keyStatus = args.KeyStatus();

    const bool is_key_down = false;
    if((int32_t)key < 256) {
        // Submit modifiers
        ImGui_ImplUwp_UpdateKeyModifiers();

        int vk = (int)key;
        if((key == (VirtualKey)VK_RETURN) && keyStatus.IsExtendedKey)
            vk = IM_VK_KEYPAD_ENTER;

        // Submit key event
        const ImGuiKey key = ImGui_ImplUwp_VirtualKeyToImGuiKey(vk);
        const int scancode = keyStatus.ScanCode;
        if(key != ImGuiKey_None)
            ImGui_ImplUWP_AddKeyEvent(key, is_key_down, vk, scancode);

        // Submit individual left/right modifier events
        if(vk == VK_SHIFT) {
            // Important: Shift keys tend to get stuck when pressed together, missing key-up events are corrected in ImGui_ImplUwp_ProcessKeyEventsWorkarounds()
            if(IsVkDown(VK_LSHIFT) == is_key_down) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_LeftShift, is_key_down, VK_LSHIFT, scancode); }
            if(IsVkDown(VK_RSHIFT) == is_key_down) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_RightShift, is_key_down, VK_RSHIFT, scancode); }
        }
        else if(vk == VK_CONTROL) {
            if(IsVkDown(VK_LCONTROL) == is_key_down) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_LeftCtrl, is_key_down, VK_LCONTROL, scancode); }
            if(IsVkDown(VK_RCONTROL) == is_key_down) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_RightCtrl, is_key_down, VK_RCONTROL, scancode); }
        }
        else if(vk == VK_MENU || !keyStatus.IsMenuKeyDown) {
            if(IsVkDown(VK_LMENU) == is_key_down) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_LeftAlt, is_key_down, VK_LMENU, scancode); }
            if(IsVkDown(VK_RMENU) == is_key_down) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_RightAlt, is_key_down, VK_RMENU, scancode); }

            if(!IsVkDown(VK_LMENU) && !IsVkDown(VK_RMENU)) { ImGui_ImplUWP_AddKeyEvent(ImGuiKey_LeftAlt, is_key_down, VK_LMENU, scancode); }
        }
    }
    if(ImGui::GetIO().WantCaptureKeyboard) {	// 这里应该是指焦点在ImGui上, 可能不在输入框里
        //args.Handled(true);
    }
}

unsigned short UTF32ToUTF16(UINT32 utf32) {
    unsigned int h, l;

    if(utf32 < 0x10000) {
        h = 0;
        l = utf32;
        return utf32;
    }
    unsigned int t = utf32 - 0x10000;
    h = (((t << 12) >> 22) + 0xD800);
    l = (((t << 22) >> 22) + 0xDC00);
    unsigned short ret = ((h << 16) | (l & 0x0000FFFF));
    return ret;
}

void characterReceived(CoreWindow const& sender, CharacterReceivedEventArgs const& args) {
    if(ImGui::GetCurrentContext() == nullptr)
        return;

    ImGuiIO& io = ImGui::GetIO();

    UINT32 code = args.KeyCode();

    io.AddInputCharacterUTF16(UTF32ToUTF16(code));

    if(ImGui::GetIO().WantCaptureKeyboard) {	// 这里应该是指焦点在ImGui上, 可能不在输入框里
        //args.Handled(true);
    }
}


///// 鼠标事件

static void OnPointerPressed(CoreWindow const& sender, PointerEventArgs const& args) {
    if(ImGui::GetCurrentContext() == nullptr) return;
    ImGuiMouseSource mouse_source = GetMouseSourceFromMessageExtraInfo();
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseSourceEvent(mouse_source);
    {
        if(args.CurrentPoint().Properties().IsLeftButtonPressed()) {
            io.AddMouseButtonEvent(0, true);
        }
        else if(args.CurrentPoint().Properties().IsRightButtonPressed()) {
            io.AddMouseButtonEvent(1, true);
        }
        else if(args.CurrentPoint().Properties().IsMiddleButtonPressed()) {
            io.AddMouseButtonEvent(2, true);
        }
    }
    if(io.WantCaptureMouseUnlessPopupClose) args.Handled(true);
}

static void OnPointerReleased(CoreWindow const& sender, PointerEventArgs const& args) {
    if(ImGui::GetCurrentContext() == nullptr) return;
    ImGuiMouseSource mouse_source = GetMouseSourceFromMessageExtraInfo();
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseSourceEvent(mouse_source);
    {

        winrt::Windows::UI::Input::PointerUpdateKind kind = args.CurrentPoint().Properties().PointerUpdateKind();
        if(kind == winrt::Windows::UI::Input::PointerUpdateKind::LeftButtonReleased) {
            // 处理左键松开情况
            io.AddMouseButtonEvent(0, false);
        }
        else if(kind == winrt::Windows::UI::Input::PointerUpdateKind::RightButtonReleased) {
            io.AddMouseButtonEvent(1, false);
        }
        else if(kind == winrt::Windows::UI::Input::PointerUpdateKind::MiddleButtonReleased) {
            io.AddMouseButtonEvent(2, false);
        }
    }
    if(io.WantCaptureMouseUnlessPopupClose) args.Handled(true);
}

static void OnPointerMoved(CoreWindow const& sender, PointerEventArgs const& args) {
    if(ImGui::GetCurrentContext() == nullptr) return;
    ImGuiMouseSource mouse_source = GetMouseSourceFromMessageExtraInfo();
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseSourceEvent(mouse_source);
    {
        const auto& position = args.CurrentPoint().Position();
        io.AddMousePosEvent(position.X, position.Y);
    }
    if(io.WantCaptureMouseUnlessPopupClose) args.Handled(true);
}

static void OnPointerWheelChanged(CoreWindow const& sender, PointerEventArgs const& args) {
    if(ImGui::GetCurrentContext() == nullptr) return;
    ImGuiMouseSource mouse_source = GetMouseSourceFromMessageExtraInfo();
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseSourceEvent(mouse_source);
    {
        io.AddMouseWheelEvent(0.f, args.CurrentPoint().Properties().MouseWheelDelta() < 0 ? -1.f : 1.f);
    }
    if(io.WantCaptureMouseUnlessPopupClose) args.Handled(true);
}

static void OnLanJie(CoreDispatcher const& sender, AcceleratorKeyEventArgs const& args) {
    if(ImGui::GetCurrentContext() == nullptr) return;
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureMouseUnlessPopupClose) args.Handled(true);
    if(io.WantCaptureMouse) args.Handled(true);
    if(io.WantCaptureKeyboard) args.Handled(true);
}




static winrt::event_token token_cookie_keydown;
static winrt::event_token token_cookie_keyup;
static winrt::event_token token_cookie_characterReceived;

static winrt::event_token token_cookie_pointPressed;
static winrt::event_token token_cookie_pointReleased;
static winrt::event_token token_cookie_pointMoved;
static winrt::event_token token_cookie_pointWheelChanged;

static winrt::event_token token_cookie_lanjie;



// on CoreWindow Thread Call
void registerCoreWindowEventHandle() {
    //// 获取corewindow
    //win = CoreWindow::GetForCurrentThread();
    //if(win == NULL) {
    //    spdlog::error(" CoreWindow::GetForCurrentThread() 获取失败,可能不是在 CoreWindow 的UI线程调用");
    //    return;
    //}

    //// 注册事件
    //token_cookie_keydown = win.KeyDown(TypedEventHandler<CoreWindow, KeyEventArgs>(&keydown));
    //token_cookie_keyup = win.KeyUp(TypedEventHandler<CoreWindow, KeyEventArgs>(&keyup));
    //token_cookie_characterReceived = win.CharacterReceived(TypedEventHandler<CoreWindow, CharacterReceivedEventArgs>(&characterReceived));
    ////win.Activate();
    CoreApplication::MainView().CoreWindow().DispatcherQueue().TryEnqueue([&]() {
        // 注册事件
        auto core = CoreWindow::GetForCurrentThread();
        win = core;
        token_cookie_keydown = core.KeyDown(TypedEventHandler<CoreWindow, KeyEventArgs>(&keydown));
        token_cookie_keyup = core.KeyUp(TypedEventHandler<CoreWindow, KeyEventArgs>(&keyup));
        token_cookie_characterReceived = core.CharacterReceived(TypedEventHandler<CoreWindow, CharacterReceivedEventArgs>(&characterReceived));

        token_cookie_pointPressed = core.PointerPressed(TypedEventHandler<CoreWindow, PointerEventArgs>(&OnPointerPressed));
        token_cookie_pointReleased = core.PointerReleased(TypedEventHandler<CoreWindow, PointerEventArgs>(&OnPointerReleased));
        token_cookie_pointMoved = core.PointerMoved(TypedEventHandler<CoreWindow, PointerEventArgs>(&OnPointerMoved));
        token_cookie_pointWheelChanged = core.PointerWheelChanged(TypedEventHandler<CoreWindow, PointerEventArgs>(&OnPointerWheelChanged));

        //token_cookie_lanjie = core.Dispatcher().AcceleratorKeyActivated(TypedEventHandler<CoreDispatcher, AcceleratorKeyEventArgs>(&OnLanJie));
    });
}

void unregisterCoreWindowEventHandle() {
    if(win) {
        // 取消注册事件
        win.KeyDown(token_cookie_keydown);
        win.KeyUp(token_cookie_keyup);
        win.CharacterReceived(token_cookie_characterReceived);

        win.PointerPressed(token_cookie_pointPressed);
        win.PointerReleased(token_cookie_pointReleased);
        win.PointerMoved(token_cookie_pointMoved);
        win.PointerWheelChanged(token_cookie_pointWheelChanged);
        
        //win.Dispatcher().AcceleratorKeyActivated(token_cookie_lanjie);
    }
}