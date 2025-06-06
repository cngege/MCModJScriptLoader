#include "imgui_uwp_wndProc.h"
#include "imgui_internal.h"
#include <imgui/imstb_textedit.h>

#define NOMINMAX
#include <windows.h>
#include <windowsx.h> // GET_X_LPARAM(), GET_Y_LPARAM()
#include "spdlog/spdlog.h"

#include <winrt/windows.ui.core.h>
#include <winrt/windows.ui.input.h>
#include <winrt/base.h>
#include <windows.system.h>

#include <winrt/windows.applicationmodel.core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/Windows.UI.Text.Core.h>
#include <winrt/Windows.UI.ViewManagement.h>
//#include "winrt/Windows.UI.Core.h"
#include <winrt/windows.system.h>
#include <winrt/windows.graphics.display.h>


//using ABI::Windows::UI::Text::Core::CoreTextServicesManager;//转为下方的使用命名空间来解决错误

using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Text::Core;
using namespace winrt::Windows::UI::Input;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::System;

//using winrt::Windows::UI::Core::CharacterReceivedEventArgs;
//using winrt::Windows::UI::Core::CoreWindow;
//using winrt::Windows::UI::Core::KeyEventArgs;
//using winrt::Windows::UI::Core::PointerEventArgs;
//using winrt::Windows::Foundation::TypedEventHandler;
//using winrt::Windows::System::VirtualKey;
//using winrt::Windows::UI::Core::CorePhysicalKeyStatus;
//using winrt::Windows::UI::Core::CoreVirtualKeyStates;



static CoreWindow win = NULL;
static CoreTextServicesManager m_manager = NULL;
static CoreTextEditContext editContext = NULL;
static InputPane inputPane = NULL;

static ImGuiID lastActiveID = 0; // 记录上一次激活的输入框ID

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

// 按下退格键的那一刻记录文本
// 松开的时候 传递到输入法
void KeyEvent_backspace(bool down);

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
        const ImGuiKey _key = ImGui_ImplUwp_VirtualKeyToImGuiKey(vk);
        if(_key == ImGuiKey_Backspace) {
            KeyEvent_backspace(true); // 去处理输入法相关
        }
        const int scancode = keyStatus.ScanCode;
        if(_key != ImGuiKey_None)
            ImGui_ImplUWP_AddKeyEvent(_key, is_key_down, vk, scancode);

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
        const ImGuiKey _key = ImGui_ImplUwp_VirtualKeyToImGuiKey(vk);
        const int scancode = keyStatus.ScanCode;
        if(_key != ImGuiKey_None)
            ImGui_ImplUWP_AddKeyEvent(_key, is_key_down, vk, scancode);
        //if(_key == ImGuiKey_Backspace) {
        //    KeyEvent_backspace(false); // 去处理输入法相关
        //}

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
    //if(io.WantCaptureMouseUnlessPopupClose) args.Handled(true);
}

static void OnPointerReleased(CoreWindow const& sender, PointerEventArgs const& args) {
    if(ImGui::GetCurrentContext() == nullptr) return;
    ImGuiMouseSource mouse_source = GetMouseSourceFromMessageExtraInfo();
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseSourceEvent(mouse_source);
    {

        PointerUpdateKind kind = args.CurrentPoint().Properties().PointerUpdateKind();
        if(kind == PointerUpdateKind::LeftButtonReleased) {
            // 处理左键松开情况
            io.AddMouseButtonEvent(0, false);
        }
        else if(kind == PointerUpdateKind::RightButtonReleased) {
            io.AddMouseButtonEvent(1, false);
        }
        else if(kind == PointerUpdateKind::MiddleButtonReleased) {
            io.AddMouseButtonEvent(2, false);
        }
    }
    //if(io.WantCaptureMouseUnlessPopupClose) args.Handled(true);
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
    //if(io.WantCaptureMouseUnlessPopupClose) args.Handled(true);
}

static void OnPointerWheelChanged(CoreWindow const& sender, PointerEventArgs const& args) {
    if(ImGui::GetCurrentContext() == nullptr) return;
    ImGuiMouseSource mouse_source = GetMouseSourceFromMessageExtraInfo();
    ImGuiIO& io = ImGui::GetIO();
    IPointerPoint pointerPoint = args.CurrentPoint();
    IPointerPointProperties pointerPointProps = pointerPoint.Properties();

    bool isHorizontalMouseWheel = pointerPointProps.IsHorizontalMouseWheel();
    if(isHorizontalMouseWheel) {
        io.AddMouseWheelEvent(-(float)pointerPointProps.MouseWheelDelta() / (float)WHEEL_DELTA, 0.0f);
    }
    else {
        io.AddMouseWheelEvent(0.0f, (float)pointerPointProps.MouseWheelDelta() / (float)WHEEL_DELTA);
        //io.AddMouseWheelEvent(0.f, pointerPointProps.MouseWheelDelta() < 0 ? -1.f : 1.f);
    }

    io.AddMouseSourceEvent(mouse_source);
    //{
    //    io.AddMouseWheelEvent(0.f, pointerPointProps.MouseWheelDelta() < 0 ? -1.f : 1.f);
    //}
    //if(io.WantCaptureMouseUnlessPopupClose) args.Handled(true);
}

void OnSelectionRequested(CoreTextEditContext const& sender, CoreTextSelectionRequestedEventArgs const& args) {
    //return;
    auto request = args.Request();
    ImGuiInputTextState* state = ImGui::GetInputTextState(lastActiveID);
    if(!state || !state->ID) {
        return;
    }
    if(request.IsCanceled()) {
        return;
    }
    if(state->HasSelection()) {
        request.Selection(CoreTextRange{
            state->GetSelectionStart(),
            state->GetSelectionEnd()
                          });
    }/*else{
        request.Selection(CoreTextRange{
            state->GetCursorPos(),
            state->GetCursorPos()
                          });
    }*/

}

std::string utf8_substr(const std::string& str,
                        uint32_t start,
                        uint32_t len = std::numeric_limits<uint32_t>::max()) {
    // 处理边界情况
    if(str.empty() || len == 0) return "";

    // 获取字节长度
    const char* ptr = str.c_str();
    size_t byteLen = str.size();

    uint32_t char_count = 0;  // 当前字符计数
    size_t start_byte = 0;  // 起始字节位置
    size_t end_byte = byteLen; // 结束字节位置
    bool start_found = false;
    uint32_t count = 0;

    // 遍历所有字节
    for(size_t i = 0; i < byteLen; ) {
        // 检查当前字符字节数
        int char_bytes = 1;
        if((ptr[i] & 0x80) == 0) {
            // 单字节字符 (0xxxxxxx)
        }
        else if((ptr[i] & 0xE0) == 0xC0) {
            // 双字节字符 (110xxxxx)
            char_bytes = 2;
        }
        else if((ptr[i] & 0xF0) == 0xE0) {
            // 三字节字符 (1110xxxx)
            char_bytes = 3;
        }
        else if((ptr[i] & 0xF8) == 0xF0) {
            // 四字节字符 (11110xxx)
            char_bytes = 4;
        }
        else {
            // 无效UTF-8序列
            throw std::runtime_error("Invalid UTF-8 sequence");
        }

        // 检查范围是否完整
        if(i + char_bytes > byteLen) {
            throw std::runtime_error("Truncated UTF-8 character");
        }

        // 检查起始位置
        if(char_count == start) {
            start_byte = i;
            start_found = true;
        }

        // 检查结束位置
        if(start_found) {
            if(count >= len) {
                end_byte = i;
                break;
            }
            count++;
        }

        // 移动到下一个字符
        i += char_bytes;
        char_count++;
    }

    // 返回截取的子串
    return str.substr(start_byte, end_byte - start_byte);
}

// 返回指定的文本范围。请注意,系统可能会要求更多文本
// 比存在于文本缓冲区中。
static void OnTextRequested(CoreTextEditContext const& sender, CoreTextTextRequestedEventArgs const& args) {
    CoreTextTextRequest request = args.Request();
    CoreTextRange requestRange = request.Range();
    ImGuiInputTextState* state = ImGui::GetInputTextState(lastActiveID);
    if(!state || !state->ID) {
        return;
    }
    if(requestRange.EndCaretPosition == -1) {
        auto hstrText = winrt::to_hstring(std::string(state->TextA.Data));
        request.Text(hstrText);
    }else{
        std::string t(state->TextA.Data);
        auto t2 = utf8_substr(t, requestRange.StartCaretPosition, requestRange.EndCaretPosition - requestRange.StartCaretPosition);
        auto hstrText = winrt::to_hstring(t2);
        request.Text(hstrText);
    }
}
// 字数转为字节偏移
static int utf8_offset(ImVector<char> strs, int offset) {
    int pos = 0;    // 字节索引位置
    int count = 0;  // 已跳过的字符计数

    while(count < offset) {
        uint8_t c = strs[pos];
        // 根据UTF-8首字节确定字符字节长度
        int char_len = 1;
        if(c >= 0xF0) char_len = 4; // 11110xxx 4字节字符
        else if(c >= 0xE0) char_len = 3; // 1110xxxx 3字节字符（如中文）
        else if(c >= 0xC0) char_len = 2; // 110xxxxx 2字节字符

        // 移动到下一个字符起始位置
        pos += char_len;
        count++;
    }

    return pos;
}

int 开始输入节点位置 = -1;
int select_min = 0, select_max = 0;
int last节长度 = 0;
int 组合输入标记流程 = 0; /*0: 没开始, 1: 开始标记 2：组合输入中 3: 结束标记组合输入进去,*/
static void OnTextUpdating(CoreTextEditContext const& sender, CoreTextTextUpdatingEventArgs const& args) {
    if(ImGui::GetCurrentContext() == nullptr)
        return;

    ImGuiIO& io = ImGui::GetIO();
    const auto incomingText = args.Text();
    const auto range = args.Range();
    int replaceStart = range.StartCaretPosition;
    int replaceLength = range.EndCaretPosition - replaceStart;

    std::string utf8Text = winrt::to_string(incomingText);
    spdlog::debug("调试输出:{}", utf8Text.c_str());
    ImGuiInputTextState* state = ImGui::GetInputTextState(lastActiveID);
    if(!state || !state->ID) {
        args.Result(CoreTextTextUpdatingResult::Failed);
        return;
    }
    
    auto stb = (STB_TexteditState*)state->Stb;    
    // 控件要他选定范围
    if(state->HasSelection()) {
        开始输入节点位置 = -1;
        
        select_min = state->GetSelectionStart();
        select_max = state->GetSelectionEnd();
    }
    if(replaceLength > 0) {
        int start = utf8_offset(state->TextA, range.StartCaretPosition);
        int end = utf8_offset(state->TextA, range.EndCaretPosition);

        spdlog::warn("输入法提示选区替换start:{}, end:{}", start, end);
        stb->select_start = start;
        stb->select_end = end;
        
        select_min = start;
        select_max = start + static_cast<int>(strlen(utf8Text.data()));
        开始输入节点位置 = -1;
    }
    else {
        if(开始输入节点位置 >= 0) {
            if(last节长度>0) {
                stb->select_start = 开始输入节点位置;
                stb->select_end = 开始输入节点位置 + last节长度;
            }
            else {
                state->ClearSelection();
                stb->cursor = 开始输入节点位置;
                // 划定下一次替换的范围
                select_min = 开始输入节点位置;
                select_max = 开始输入节点位置 + static_cast<int>(strlen(utf8Text.data()));
                开始输入节点位置 = -1;
            }
        }
        else {
            // 否则表示是选择一个范围
            stb->select_start = select_min;
            stb->select_end = select_max;
            // 依旧划定下一次替换范围
            select_min = select_min;
            select_max = select_min + static_cast<int>(strlen(utf8Text.data()));
        }
    }
    last节长度 = static_cast<int>(strlen(utf8Text.data()));
    if(/*utf8Text == "" || */utf8Text.empty()) {// 两个都为true
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Backspace, true);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Backspace, false);
    }
    else {
        io.AddInputCharactersUTF8(utf8Text.data());
    }
    //state->CursorAnimReset();
    args.Result(CoreTextTextUpdatingResult::Succeeded);
}

static void OnCompositionStarted(CoreTextEditContext const& sender, CoreTextCompositionStartedEventArgs const& args) {
    ImGuiInputTextState* state = ImGui::GetInputTextState(lastActiveID);
    if(!state || !state->ID) {
        spdlog::error("OnCompositionStarted !start||!state->ID");
        return;
    }
    if(state->HasSelection()) {
        开始输入节点位置 = -1;
    }
    else {
        开始输入节点位置 = state->GetCursorPos();
    }
    last节长度 = 0;
    
    spdlog::debug("开始输入节点位置:{}", 开始输入节点位置);
}

static void OnCompositionCompleted(CoreTextEditContext const& sender, CoreTextCompositionCompletedEventArgs const& args) {
    // 输入中文组合完成
    //auto& io = ImGui::GetIO();
    ImGuiInputTextState* state = ImGui::GetInputTextState(lastActiveID);
    if(!state || !state->ID) {
        return;
    }
    //auto stb = (STB_TexteditState*)state->Stb;
    if(state->HasSelection())
        开始输入节点位置 = -1;
    else
        开始输入节点位置 = state->GetCursorPos();
    last节长度 = 0;
}
/**
 * @brief 同步更新选区
 * @param sender 
 * @param args 
 */
static void OnSelectionUpdating(CoreTextEditContext const& sender, CoreTextSelectionUpdatingEventArgs const & args) {
    ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetActiveID());
    if(state) {
        // 更新系统选区范围
        auto select = args.Selection();
        select.StartCaretPosition = state->HasSelection()? std::min(state->GetSelectionStart(), state->GetSelectionEnd()) : state->GetCursorPos();
        select.EndCaretPosition = state->HasSelection() ? std::max(state->GetSelectionStart(), state->GetSelectionEnd()) : state->GetCursorPos();
        args.Result(CoreTextSelectionUpdatingResult::Succeeded);
    }
    args.Result(CoreTextSelectionUpdatingResult::Failed);
}
/**
 * @brief 更新输入法小窗口位置
 * @param sender 
 * @param args 
 */
static void OnLayoutRequested(CoreTextEditContext const& sender, CoreTextLayoutRequestedEventArgs const& args) {
    auto req = args.Request();
    if(!req) {
        spdlog::warn("args.Request() 为空 {}:{}", __FILE__, __LINE__);
        return;
    }
    if(lastActiveID != 0) {
        ImVec2 pos = /*ImGui::GetItemRectMin()*/ImGui::GetMousePos();
        ImVec2 size = ImGui::GetItemRectSize();

        POINT pt = { static_cast<LONG>(pos.x), static_cast<LONG>(pos.y + 15) };
        //auto rect = win.GetForCurrentThread().Bounds();
        auto window = (HWND)FindWindowA(nullptr, (LPCSTR)"Minecraft");
        auto childwindow = (HWND)FindWindowExA(window, NULL, NULL, (LPCSTR)"Minecraft");
        if(window == NULL || childwindow == NULL) {
            spdlog::error("未能获取窗口句柄 {}:{}", __FILE__, __LINE__);
            req.LayoutBounds().TextBounds({ 10.0f,10.0f,10.0f,10.0f });
            return;
        }
        ::ClientToScreen(childwindow/*window，childwindow*/, &pt);
        //double scaleFactor = winrt::Windows::Graphics::Display::DisplayInformation::GetForCurrentView().RawPixelsPerViewPixel();
        Rect rect{
            static_cast<float>(pt.x),
            static_cast<float>(pt.y),
            static_cast<float>(size.x),
            static_cast<float>(size.y)};

        req.LayoutBounds().TextBounds(rect);
    }
    else {
        req.LayoutBounds().TextBounds({ 10.0f,10.0f,10.0f,10.0f });
    }
}


bool 在输入框按下了退格键 = false;
void ImGui_Uwp_EveryUpdate_Frame() {
    static bool wantinput = false;
    auto& io = ImGui::GetIO();
    if(io.WantTextInput && io.WantCaptureMouseUnlessPopupClose) {
        if(!wantinput) {
            try {
                if(!editContext) {
                    return;
                }
                auto _lastActiveID = ImGui::GetActiveID();
                // 首次焦点时通知当前文本内容
                if(ImGuiInputTextState* state = ImGui::GetInputTextState(_lastActiveID)) {
                    lastActiveID = _lastActiveID;
                    wantinput = true;
                    CoreApplication::MainView().CoreWindow().DispatcherQueue().TryEnqueue([=]() {
                        editContext.NotifyLayoutChanged();
                        editContext.NotifyFocusEnter();
                        editContext.NotifyTextChanged(
                            CoreTextRange{ state->GetCursorPos(), /*static_cast<int32_t>(state->TextA.Size)*/state->GetCursorPos() + 1},
                            state->TextLen,
                            CoreTextRange{ state->GetSelectionStart(), state->GetSelectionEnd()}
                        );
                        // 4. 重置选择状态 (触发 SelectionUpdating)
                        editContext.NotifySelectionChanged(CoreTextRange{
                            state->HasSelection() ? state->GetSelectionStart(): state->GetCursorPos(),
                            state->HasSelection() ? state->GetSelectionEnd() : state->GetCursorPos()});
                        
                        //inputPane.TryShow();
                    });
                }
                else {
                    spdlog::error("ImGui::GetInputTextState(lastActiveID) == nullptr");
                }
            }
            catch(std::exception& err) {
                spdlog::error("err:{}",err.what());
            }
        }
    }
    else {
        if(wantinput) {
            wantinput = false;
            CoreApplication::MainView().CoreWindow().DispatcherQueue().TryEnqueue([&]() {
                if(editContext) {
                    editContext.NotifyFocusLeave();
                    //inputPane.TryHide();
                }
            });
            lastActiveID = 0;
        }
    }
    if(在输入框按下了退格键) {
        if(ImGui::IsKeyReleased(ImGuiKey_Backspace)) {
            KeyEvent_backspace(false);
        }
    }
}

// 比较函数
static void CompareTextChanges(
    const ImVector<char>& oldText,
    const ImVector<char>& newText,
    CoreTextRange& modifiedRange,
    int32_t& newLength) {
    // 1. 计算公共前缀长度
    uint32_t prefixLen = 0;
    const uint32_t minSize = static_cast<uint32_t>(std::min(oldText.size(), newText.size()));//考虑到应该不会小于0，所以强转uint32_t
    while(prefixLen < minSize && oldText[prefixLen] == newText[prefixLen]) {
        ++prefixLen;
    }

    // 2. 计算公共后缀长度
    uint32_t suffixLen = 0;
    uint32_t oldIndex = static_cast<uint32_t>(oldText.size());
    uint32_t newIndex = static_cast<uint32_t>(newText.size());

    // 从后向前比较，直到遇到前缀边界或差异
    while(oldIndex > prefixLen && newIndex > prefixLen) {
        if(oldText[oldIndex - 1] != newText[newIndex - 1]) break;
        --oldIndex;
        --newIndex;
        ++suffixLen;
    }

    // 3. 计算修改范围
    modifiedRange = {
        static_cast<int>(prefixLen),  // 修改起始位置
        static_cast<int>(oldText.size() - suffixLen)  // 修改结束位置
    };

    // 4. 计算新增文本长度
    const uint32_t newContentLength =
        static_cast<uint32_t>(newText.size()) - prefixLen - suffixLen;
    newLength = newContentLength;
}

// 记录的数据 暂时不考虑跨输入框的操作
ImVector<char> 按下时的文本 = {};

static void KeyEvent_backspace(bool down) {
    if(!down) {
        在输入框按下了退格键 = false;
    }
    ImGuiInputTextState* state = ImGui::GetInputTextState(lastActiveID);
    if(!state || !state->ID) {
        在输入框按下了退格键 = false;
        return;//不在输入框 不管
    }
    if(down) {
        // 记录文字
        按下时的文本 = state->TextA;
        在输入框按下了退格键 = true;
        // 重置输入法文本
        CoreApplication::MainView().CoreWindow().DispatcherQueue().TryEnqueue([=]() {
            editContext.NotifyFocusLeave();
        });
    }
    else {
        
        在输入框按下了退格键 = false;
        // 通知文本变化
        CoreApplication::MainView().CoreWindow().DispatcherQueue().TryEnqueue([=]() {
            editContext.NotifyLayoutChanged();
            editContext.NotifyFocusEnter();
            editContext.NotifyTextChanged(
                CoreTextRange{ 0, 1},
                state->TextLen,
                CoreTextRange{ state->GetCursorPos(), state->GetCursorPos() }
            );
            // 4. 重置选择状态 (触发 SelectionUpdating)
            editContext.NotifySelectionChanged(CoreTextRange{
                state->GetCursorPos(),
                state->GetCursorPos() });

            //inputPane.TryShow();
        });

        return;
        在输入框按下了退格键 = false;
        // 通知
        // 比较两个文本

        CoreTextRange 变化位置{};
        int  变化长度 = 0;
        CompareTextChanges(按下时的文本, state->TextA, 变化位置, 变化长度);

        CoreTextRange 最后位置{};
        if(state->HasSelection())
        {
            最后位置 = { state->GetSelectionStart(), state->GetSelectionEnd() };
        }
        else {
            最后位置 = { state->GetCursorPos(), state->GetCursorPos() };
        }
        editContext.NotifyTextChanged(
            变化位置,
            变化长度,
            最后位置
        );

        //重置选择状态 (触发 SelectionUpdating)
        editContext.NotifySelectionChanged(CoreTextRange{
            最后位置.StartCaretPosition,最后位置.EndCaretPosition});
    }

}

static winrt::event_token token_cookie_keydown;
static winrt::event_token token_cookie_keyup;
static winrt::event_token token_cookie_characterReceived;

static winrt::event_token token_cookie_pointPressed;
static winrt::event_token token_cookie_pointReleased;
static winrt::event_token token_cookie_pointMoved;
static winrt::event_token token_cookie_pointWheelChanged;

static winrt::event_token token_cookie_lanjie;
// 输入法输入测试

static winrt::event_token token_cookie_IMEChangedLg;


static winrt::event_token token_cookie_IMETextRequested;
static winrt::event_token token_cookie_IMETextUpdating;
static winrt::event_token token_cookie_IMESelectionUpdating;
static winrt::event_token token_cookie_IMESelectionRequested;
static winrt::event_token token_cookie_IMELayoutRequested;
static winrt::event_token token_cookie_IMECompositionStarted;
static winrt::event_token token_cookie_IMECompositionCompleted;

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
        if(win) {
            token_cookie_keydown = core.KeyDown(TypedEventHandler<CoreWindow, KeyEventArgs>(&keydown));
            token_cookie_keyup = core.KeyUp(TypedEventHandler<CoreWindow, KeyEventArgs>(&keyup));
            //token_cookie_characterReceived = core.CharacterReceived(TypedEventHandler<CoreWindow, CharacterReceivedEventArgs>(&characterReceived));

            token_cookie_pointPressed = core.PointerPressed(TypedEventHandler<CoreWindow, PointerEventArgs>(&OnPointerPressed));
            token_cookie_pointReleased = core.PointerReleased(TypedEventHandler<CoreWindow, PointerEventArgs>(&OnPointerReleased));
            token_cookie_pointMoved = core.PointerMoved(TypedEventHandler<CoreWindow, PointerEventArgs>(&OnPointerMoved));
            token_cookie_pointWheelChanged = core.PointerWheelChanged(TypedEventHandler<CoreWindow, PointerEventArgs>(&OnPointerWheelChanged));
            //token_cookie_lanjie = core.Dispatcher().AcceleratorKeyActivated(TypedEventHandler<CoreDispatcher, AcceleratorKeyEventArgs>(&OnLanJie));
        }

        // 尝试解决输入法问题
        m_manager = CoreTextServicesManager::GetForCurrentView();
        token_cookie_IMEChangedLg = m_manager.InputLanguageChanged([=](CoreTextServicesManager const& sender, auto const& args) {
            //spdlog::debug("切换语言，我被调用");
            
        });
        inputPane = InputPane::GetForCurrentView();
        editContext = m_manager.CreateEditContext();
        if(editContext) {
            // 必须设置：自动显示输入面板（软键盘）
            editContext.InputPaneDisplayPolicy(CoreTextInputPaneDisplayPolicy::Automatic);
            editContext.InputScope(CoreTextInputScope::Text);

            token_cookie_IMETextRequested = editContext.TextRequested(TypedEventHandler<CoreTextEditContext, CoreTextTextRequestedEventArgs>(&OnTextRequested));
            token_cookie_IMELayoutRequested = editContext.LayoutRequested(TypedEventHandler<CoreTextEditContext, CoreTextLayoutRequestedEventArgs>(&OnLayoutRequested));
            token_cookie_IMESelectionUpdating = editContext.SelectionUpdating(TypedEventHandler<CoreTextEditContext, CoreTextSelectionUpdatingEventArgs>(&OnSelectionUpdating));
            //token_cookie_IMESelectionRequested = editContext.SelectionRequested(TypedEventHandler<CoreTextEditContext, CoreTextSelectionRequestedEventArgs>(&OnSelectionRequested));

            token_cookie_IMETextUpdating = editContext.TextUpdating(TypedEventHandler<CoreTextEditContext, CoreTextTextUpdatingEventArgs>(&OnTextUpdating));
            token_cookie_IMECompositionStarted = editContext.CompositionStarted(TypedEventHandler<CoreTextEditContext, CoreTextCompositionStartedEventArgs>(&OnCompositionStarted));
            token_cookie_IMECompositionCompleted = editContext.CompositionCompleted(TypedEventHandler<CoreTextEditContext, CoreTextCompositionCompletedEventArgs>(&OnCompositionCompleted));
        }
        else {
            spdlog::error("编辑器上下文创建失败");
        }
    });

}

void unregisterCoreWindowEventHandle() {
    CoreApplication::MainView().CoreWindow().DispatcherQueue().TryEnqueue([=]() {
        if(win) {
            // 取消注册事件
            win.KeyDown(token_cookie_keydown);
            win.KeyUp(token_cookie_keyup);
            //win.CharacterReceived(token_cookie_characterReceived);

            win.PointerPressed(token_cookie_pointPressed);
            win.PointerReleased(token_cookie_pointReleased);
            win.PointerMoved(token_cookie_pointMoved);
            win.PointerWheelChanged(token_cookie_pointWheelChanged);

            //win.Dispatcher().AcceleratorKeyActivated(token_cookie_lanjie);
        }
        if(m_manager) {
            editContext.NotifyFocusLeave();
            m_manager.InputLanguageChanged(token_cookie_IMEChangedLg);
        }
        if(editContext) {
            editContext.TextRequested(token_cookie_IMETextRequested);
            editContext.TextUpdating(token_cookie_IMETextUpdating);
            editContext.SelectionUpdating(token_cookie_IMESelectionUpdating);
            //editContext.SelectionRequested(token_cookie_IMESelectionRequested);

            editContext.LayoutRequested(token_cookie_IMELayoutRequested);
            editContext.CompositionStarted(token_cookie_IMECompositionStarted);
            editContext.CompositionCompleted(token_cookie_IMECompositionCompleted);
        }
    });
}