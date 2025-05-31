#include "../JSManager.h"
#include "class/imguiclass.h"
#include "../client/ModManager.h"
#include "imgui/imgui.h"
#include "imgui_toggle/imgui_toggle.h"
#include "../nativePoint/nativePointClass.h"
#include <list>

static JSValue js_imguiDemoWindow(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if(argc >= 1) {
        auto ret_str = JSTool::ReferenceBoolCall(argv[0], [&](bool* v) {
            ImGui::ShowDemoWindow(v);
        });
        if(!ret_str.empty()) {
            return JS_ThrowTypeError(ctx, ret_str.c_str());
        }
        return JS_UNDEFINED;
    }
    ImGui::ShowDemoWindow();
    return JS_UNDEFINED;
}

static JSValue js_imgui_Begin(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    //1   字符串 标题
    //[2] bool指针 - 引用值|bool isopen
    //[3] int flag

    bool ret = false;
    std::optional<std::string> label;
    //v
    std::optional<int> flags = 0;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse(false)
        .Parse(flags)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    if(argc >= 2) {
        auto ret_str = JSTool::ReferenceBoolCall(argv[1], [&](bool* v) {
            ret = ImGui::Begin(label->c_str(), v, *flags);
        });
        if(!ret_str.empty()) {
            return JS_ThrowTypeError(ctx, ret_str.c_str());
        }
    }
    else {
        ret = ImGui::Begin(label->c_str());
    }

    return JS_NewBool(ctx, ret);
}

static JSValue js_imgui_End(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    ImGui::End();
    return JS_UNDEFINED;
}

static JSValue js_imgui_Text(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    // 1 字符串
    if(argc < 1) return JS_ThrowTypeError(ctx, "至少需要一个参数,类型为String");
    std::string _str = "";
    for(int i = 0; i < argc; i++) {
        auto str = JSTool::toString(argv[i]);
        if(!str) return JS_ThrowTypeError(ctx, "参数%d无法解析为字符串", i);
        if(i) {
            _str += " ";
        }
        _str += *str;
    }
    ImGui::Text(_str.c_str());
    return JS_UNDEFINED;
}

static JSValue js_imgui_Button(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    // 1 字符串
    // [2] ImVer2

    std::optional<std::string> str;
    std::optional<ImVec2> size = ImVec2(0,0);

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(str)
        .Parse(size)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }
    bool ret = ImGui::Button(str->c_str(), size.value());
    return JS_NewBool(ctx, ret);
}

static JSValue js_imgui_Checkbox(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    //1   字符串 标题
    //[2] bool指针 - 引用值|bool ischeck

    bool ret = false;
    std::optional<std::string> label;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse()
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    auto ret_str = JSTool::ReferenceBoolCall(argv[1], [&](bool* v) {
        ret = ImGui::Checkbox(label->c_str(), v);
    });
    if(!ret_str.empty()) {
        return JS_ThrowTypeError(ctx, ret_str.c_str());
    }
    return JS_NewBool(ctx, ret);
}

static JSValue js_imgui_Toggle(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    //1   字符串 标题
    //[2] bool指针 - 引用值|bool ischeck
    //[2] {x, y}  - 大小|bool 宽高
    bool ret = false;
    std::optional<std::string> label;
    //v
    std::optional<ImVec2> size = ImVec2();

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse()
        .Parse(size)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    auto ret_str = JSTool::ReferenceBoolCall(argv[1], [&](bool* v) {
        ret = ImGui::Toggle(label->c_str(), v, *size);
    });
    if(!ret_str.empty()) {
        return JS_ThrowTypeError(ctx, ret_str.c_str());
    }
    return JS_NewBool(ctx, ret);
}


static JSValue js_imgui_InputInt(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {

    bool ret = false;
    std::optional<std::string> label;
    //V
    std::optional<int> step = 1;
    std::optional<int> step_fast = 100;
    std::optional<int> flags = 0;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse()
        .Parse(step)
        .Parse(step_fast)
        .Parse(flags)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    auto ret_str = JSTool::ReferenceIntCall(argv[1], [&](int* v) {
        ret = ImGui::InputInt(label.value().c_str(), v, *step, *step_fast, *flags);
    });
    if(!ret_str.empty()) {
        return JS_ThrowTypeError(ctx, ret_str.c_str());
    }
    return JS_NewBool(ctx, ret);
}

static JSValue js_imgui_InputFloat(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {

    bool ret = false;
    std::optional<std::string> label;
    //V
    std::optional<float> step = 1.f;
    std::optional<float> step_fast = 100.f;
    std::optional<std::string> format = "%.3f";
    std::optional<int> flags = 0;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse()
        .Parse(step)
        .Parse(step_fast)
        .Parse(format)
        .Parse(flags)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }
    auto ret_str = JSTool::ReferenceFloatCall(argv[1], [&](float* v) {
        ret = ImGui::InputFloat(label.value().c_str(), v, *step, *step_fast, format->c_str(), *flags);
    });
    if(!ret_str.empty()) {
        return JS_ThrowTypeError(ctx, ret_str.c_str());
    }
    return JS_NewBool(ctx, ret);
}


static JSValue js_imgui_SliderInt(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {

    bool ret = false;
    std::optional<std::string> label;
    //int fv;
    std::optional<int> min;
    std::optional<int> max;
    std::optional<std::string> format = "%d";
    std::optional<int> flags = 0;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse()
        .Parse(min)
        .Parse(max)
        .Parse(format)
        .Parse(flags)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    auto ret_str = JSTool::ReferenceIntCall(argv[1], [&](int* v) {
        ret = ImGui::SliderInt(label.value().c_str(), v, *min, *max, format.value().c_str(), *flags);
    });
    if(!ret_str.empty()) {
        return JS_ThrowTypeError(ctx, ret_str.c_str());
    }
    return JS_NewBool(ctx, ret);
}

// SliderFloat控件
static JSValue js_imgui_SliderFloat(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    bool ret = false;
    std::optional<std::string> label;
    std::optional<float> min;
    std::optional<float> max;
    std::optional<std::string> format = "%.3f";
    std::optional<int> flags = 0;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse()
        .Parse(min)
        .Parse(max)
        .Parse(format)
        .Parse(flags)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    auto ret_str = JSTool::ReferenceFloatCall(argv[1], [&](float* v) {
        ret = ImGui::SliderFloat(label->c_str(), v, *min, *max, format->c_str(), *flags);
    });
    if(!ret_str.empty()) {
        return JS_ThrowTypeError(ctx, ret_str.c_str());
    }
    return JS_NewBool(ctx, ret);
}

// Combo下拉框
static JSValue js_imgui_Combo(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    bool ret = false;
    std::optional<std::string> label;
    std::optional<std::vector<std::string>> items;
    std::optional<int> popup_max_height = -1;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse()
        .Parse(items)
        .Parse(popup_max_height)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    // 转换字符串数组到C格式
    std::vector<const char*> c_items;
    for(auto& s : *items) {
        c_items.push_back(s.c_str());
    }

    auto ret_str = JSTool::ReferenceIntCall(argv[1], [&](int* v) {
        //ret = ImGui::Combo(label->c_str(), v, c_items.data(), IM_ARRAYSIZE(c_items.data()), *popup_max_height);
        ret = ImGui::Combo(label->c_str(), v, c_items.data(), JSTool::iMin(c_items.size(), INT_MAX), *popup_max_height);
    });
    if(!ret_str.empty()) {
        return JS_ThrowTypeError(ctx, ret_str.c_str());
    }
    return JS_NewBool(ctx, ret);
}

// ColorEdit3颜色选择器
static JSValue js_imgui_ColorEdit3(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    bool ret = false;
    std::optional<std::string> label;
    std::optional<int> flags = 0;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse()
        .Parse(flags)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    auto ret_str = JSTool::ReferenceVec3PropCall(argv[1], [&](float* v) {
        ret = ImGui::ColorEdit3(label->c_str(), v, *flags);
    });
    if(!ret_str.empty()) {
        return JS_ThrowTypeError(ctx, ret_str.c_str());
    }
    return JS_NewBool(ctx, ret);
}

// RadioButton单选按钮
static JSValue js_imgui_RadioButton(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if(argc == 2) {
        std::optional<std::string> label;
        std::optional<bool> active;

        auto retV = JSTool::createParseParameter(argc, argv)
            .Parse(label)
            .Parse(active)
            .Build();
        if(!retV.empty()) {
            return JS_ThrowTypeError(ctx, retV.c_str());
        }

        bool ret = ImGui::RadioButton(label->c_str(), *active);
        return JS_NewBool(ctx, ret);
    }
    else { // 三参数
        std::optional<std::string> label;
        std::optional<int> v_button;

        bool ret = false;
        auto retV = JSTool::createParseParameter(argc, argv)
            .Parse(label)
            .Parse()
            .Parse(v_button)
            .Build();
        if(!retV.empty()) {
            return JS_ThrowTypeError(ctx, retV.c_str());
        }

        auto ret_str = JSTool::ReferenceIntCall(argv[1], [&](int* v) {
            ret = ImGui::RadioButton(label->c_str(), v, *v_button);
        });
        return JS_NewBool(ctx, ret);
    }
}

// ProgressBar进度条
static JSValue js_imgui_ProgressBar(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    std::optional<float> fraction;
    std::optional<ImVec2> size_arg = ImVec2(-FLT_MIN, 0);
    std::optional<std::string> overlay = std::string{};

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(fraction)
        .Parse(size_arg)
        .Parse(overlay)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    ImGui::ProgressBar(*fraction, *size_arg, overlay->empty() ? NULL : overlay->c_str());
    return JS_UNDEFINED;
}

// ListBox列表选择框
//    export function ListBox(label: string, current_item: (v?:number)=>number, items: string[], height_in_items?: number): Boolean;
static JSValue js_imgui_ListBox(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    bool ret = false;
    std::optional<std::string> label;
    std::optional<std::vector<std::string>> items;
    std::optional<int> height_in_items = -1;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse()
        .Parse(items)
        .Parse(height_in_items)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    std::vector<const char*> c_items;
    for(auto& s : *items) c_items.push_back(s.c_str());

    auto ret_str = JSTool::ReferenceIntCall(argv[1], [&](int* v) {
        ret = ImGui::ListBox(label->c_str(), v, c_items.data(), JSTool::iMin(c_items.size(),INT_MAX), *height_in_items);
    });
    if(!ret_str.empty()) {
        return JS_ThrowTypeError(ctx, ret_str.c_str());
    }
    return JS_NewBool(ctx, ret);
}

// Selectable可选项
// imgui.Selectable(label, (_ = str)=>str = _, flags:number, size?:vec_2);
//   如果第二个参数是bool值 一下类似于没点的ul->li列表 可以控制选中效果，像按钮一样的点击效果
// imgui.Selectable(label, seleted?:bool,flags?:number,size?:vec_2)
//   如果第二个参数是指针类型，那么这个选择列表就是一个可多选的列表，简单来说，如果是指针，那么点击就会自动改变它的值
//    export function Selectable(label: string, selected: (v?:boolean)=>boolean, flags?: number, size?: vec_2): Boolean;
//    export function Selectable(label: string, selected?: boolean, flags?: number, size?: vec_2): Boolean;
static JSValue js_imgui_Selectable(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    // 两种形式：1.带选中状态 2.仅返回点击状态
    if(argc >= 2 && JSTool::isFun(argv[1])) {
        bool ret = false;
        std::optional<std::string> label;
        std::optional<int> flags = 0;
        std::optional<ImVec2> size = ImVec2(0, 0);

        auto retV = JSTool::createParseParameter(argc, argv)
            .Parse(label)
            .Parse()
            .Parse(flags)
            .Parse(size)
            .Build();
        if(!retV.empty()) {
            return JS_ThrowTypeError(ctx, retV.c_str());
        }

        auto ret_str = JSTool::ReferenceBoolCall(argv[1], [&](bool* v) {
            ret = ImGui::Selectable(label->c_str(), v, *flags, *size);
        });
        if(!ret_str.empty()) {
            return JS_ThrowTypeError(ctx, ret_str.c_str());
        }
        return JS_NewBool(ctx, ret);
    }
    else {
        std::optional<std::string> label;
        std::optional<bool> selected = false;
        std::optional<int> flags = 0;
        std::optional<ImVec2> size = ImVec2(0, 0);

        auto retV = JSTool::createParseParameter(argc, argv)
            .Parse(label)
            .Parse(selected)
            .Parse(flags)
            .Parse(size)
            .Build();
        if(!retV.empty()) {
            return JS_ThrowTypeError(ctx, retV.c_str());
        }

        bool ret = ImGui::Selectable(label->c_str(), *selected, *flags, *size);
        return JS_NewBool(ctx, ret);
    }
}

// InputText文本输入(InputTextWithHint 下面实现这个
// ImGui.InputText("label", (_ = str: string)=>str = _,bufsize?:number, flags?:number);
//    export function InputText(label: string,size: number, buf: (v?:string)=>string, flags?: number): Boolean;
static JSValue js_imgui_InputText(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    bool ret = false;
    std::optional<std::string> label;
    std::optional<size_t> size;
    std::optional<int> flags = 0;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse(size)
        .Parse()
        .Parse(flags)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    auto ret_str = JSTool::ReferenceStringCall(argv[2],*size, [&](char* buf) {
        ret = ImGui::InputText(label->c_str(), buf, *size, *flags);
    });
    if(!ret_str.empty()) {
        return JS_ThrowTypeError(ctx, ret_str.c_str());
    }
    return JS_NewBool(ctx, ret);
}

// TreeNode树节点
//    export function TreeNode(label: string, flags?: number): Boolean;
static JSValue js_imgui_TreeNode(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    std::optional<std::string> label;
    std::optional<int> flags = 0;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse(flags)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    bool ret = ImGui::TreeNodeEx(label->c_str(), *flags);
    return JS_NewBool(ctx, ret);
}

// TreePop树节点出栈
//    export function TreePop():void;
static JSValue js_imgui_TreePop(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    ImGui::TreePop();
    return JS_UNDEFINED;
}

// CollapsingHeader折叠头
//    export function CollapsingHeader(label: string, flags?: number): Boolean;
static JSValue js_imgui_CollapsingHeader(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    std::optional<std::string> label;
    std::optional<int> flags = 0;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(label)
        .Parse(flags)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    bool ret = ImGui::CollapsingHeader(label->c_str(), *flags);
    return JS_NewBool(ctx, ret);
}

// 布局控件 画一条线
//    export function Separator():void;
static JSValue js_imgui_Separator(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    ImGui::Separator();
    return JS_UNDEFINED;
}

// 布局控件
//    export function Separator():void;
static JSValue js_imgui_SeparatorText(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if(argc >= 1) {
        auto toStr = JSTool::toString(argv[0]);
        if(toStr) {
            ImGui::SeparatorText(toStr->c_str());
            return JS_UNDEFINED;
        }
    }
    return JS_ThrowTypeError(ctx, "参数不足或无法转为字符串");
}

// 不换行
//    export function SameLine(offset_from_start_x?:number, spacing?:number):void;
static JSValue js_imgui_SameLine(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    std::optional<float> offset = 0.0f;
    std::optional<float> spacing = -1.0f;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(offset)
        .Parse(spacing)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    ImGui::SameLine(*offset, *spacing);
    return JS_UNDEFINED;
}

// 空出一定空间
//    export function Spacing():void;
static JSValue js_imgui_Spacing(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    ImGui::Spacing();
    return JS_UNDEFINED;
}

// 设置光标位置
//    export function SetCursorPos(pos:vec_2):void;
static JSValue js_imgui_SetCursorPos(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    std::optional<ImVec2> pos;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(pos)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    ImGui::SetCursorPos(*pos);
    return JS_UNDEFINED;
}

static JSValue js_imgui_IsItemFocused(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewBool(ctx, ImGui::IsItemFocused());
}

static JSValue js_imgui_IsItemHovered(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if(argc == 0) {
        return JS_NewBool(ctx, ImGui::IsItemHovered());
    }
    else {
        auto toInt = JSTool::toInt(argv[0]);
        if(!toInt) return JS_ThrowTypeError(ctx, "参数1 无法转为Int");
        return JS_NewBool(ctx, ImGui::IsItemHovered(*toInt));
    }
}

static JSValue js_imgui_IsItemActive(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewBool(ctx, ImGui::IsItemActive());
}

static JSValue js_imgui_IsItemEdited(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewBool(ctx, ImGui::IsItemEdited());
}

static JSValue js_imgui_IsItemActivated(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewBool(ctx, ImGui::IsItemActivated());
}

static JSValue js_imgui_IsItemDeactivated(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewBool(ctx, ImGui::IsItemDeactivated());
}

static JSValue js_imgui_IsItemDeactivatedAfterEdit(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewBool(ctx, ImGui::IsItemDeactivatedAfterEdit());
}

static JSValue js_imgui_IsItemVisible(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewBool(ctx, ImGui::IsItemVisible());
}

static JSValue js_imgui_IsItemClicked(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if(argc == 0) {
        return JS_NewBool(ctx, ImGui::IsItemClicked());
    }
    else {
        auto toInt = JSTool::toInt(argv[0]);
        if(!toInt) return JS_ThrowTypeError(ctx, "参数1 无法转为Int");
        return JS_NewBool(ctx, ImGui::IsItemClicked(*toInt));
    }
}

static JSValue js_imgui_IsItemToggledOpen(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewBool(ctx, ImGui::IsItemToggledOpen());
}

static JSValue js_imgui_GetItemRectMin(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    ImVec2 xy = ImGui::GetItemRectMin();
    JSValue obj = JS_NewObject(ctx);
    if(!JSTool::setPropXY(obj, { xy.x, xy.y })) {
        return JS_ThrowTypeError(ctx, "创建一个对象并设置属性 x y时失败");
    }
    return obj;
}

static JSValue js_imgui_GetItemRectMax(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    ImVec2 xy = ImGui::GetItemRectMax();
    JSValue obj = JS_NewObject(ctx);
    if(!JSTool::setPropXY(obj, { xy.x, xy.y })) {
        return JS_ThrowTypeError(ctx, "创建一个对象并设置属性 x y时失败");
    }
    return obj;
}

static JSValue js_imgui_GetItemRectSize(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    ImVec2 xy = ImGui::GetItemRectSize();
    JSValue obj = JS_NewObject(ctx);
    if(!JSTool::setPropXY(obj, { xy.x, xy.y })) {
        return JS_ThrowTypeError(ctx, "创建一个对象并设置属性 x y时失败");
    }
    return obj;
}

// 设置剪切板文本
//    export function SetClipboardText(pos:vec_2):void;
static JSValue js_imgui_SetClipboardText(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    std::optional<std::string> text;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(text)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    ImGui::SetClipboardText(text->c_str());
    return JS_UNDEFINED;
}



// 获取剪切板文本
//    export function GetClipboardText():string;
static JSValue js_imgui_GetClipboardText(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    std::string clip_t = ImGui::GetClipboardText();
    return JSTool::fromString(clip_t);
}

static JSValue js_imgui_GetForegroundDrawList(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    
    JSValue obj = JS_NewObjectClass(ctx, JSForegroundDrawList::getForegroundDrawListID());
    JS_SetOpaque(obj, draw);
    return obj;
}

static JSValue js_imgui_GetIO(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    ImGuiIO* io = &ImGui::GetIO();
    JSValue obj = JS_NewObjectClass(ctx, JSImGuiIO::getImGuiIOID());
    JS_SetOpaque(obj, io);
    return obj;
}



static const JSCFunctionListEntry js_imgui_funcs[] = {
     //JS_CFUNC_DEF("Get", 1, js_imguiDemoWindow),
        JS_CFUNC_DEF2("ShowDemoWindow", 0, js_imguiDemoWindow),
        JS_CFUNC_DEF2("Begin", 0, js_imgui_Begin),
        JS_CFUNC_DEF2("End", 0, js_imgui_End),
        JS_CFUNC_DEF2("Text", 1, js_imgui_Text),
        JS_CFUNC_DEF2("Button", 2, js_imgui_Button),
        JS_CFUNC_DEF2("Checkbox", 2, js_imgui_Checkbox),
        JS_CFUNC_DEF2("Toggle", 2, js_imgui_Toggle),
        JS_CFUNC_DEF2("InputInt", 2, js_imgui_InputInt),
        JS_CFUNC_DEF2("InputFloat", 2, js_imgui_InputFloat),
        JS_CFUNC_DEF2("SliderInt", 4, js_imgui_SliderInt),
        JS_CFUNC_DEF2("SliderFloat", 4, js_imgui_SliderFloat),
        JS_CFUNC_DEF2("Combo", 3, js_imgui_Combo),
        JS_CFUNC_DEF2("ColorEdit3", 2, js_imgui_ColorEdit3),
        JS_CFUNC_DEF2("RadioButton", 2, js_imgui_RadioButton),
        JS_CFUNC_DEF2("ProgressBar", 1, js_imgui_ProgressBar),
        JS_CFUNC_DEF2("ListBox", 3, js_imgui_ListBox),
        JS_CFUNC_DEF2("Selectable", 2, js_imgui_Selectable),
        JS_CFUNC_DEF2("InputText", 2, js_imgui_InputText),
        JS_CFUNC_DEF2("TreeNode", 1, js_imgui_TreeNode),
        JS_CFUNC_DEF2("TreePop", 0, js_imgui_TreePop),
        JS_CFUNC_DEF2("CollapsingHeader", 1, js_imgui_CollapsingHeader),
        JS_CFUNC_DEF2("Separator", 0, js_imgui_Separator),
        JS_CFUNC_DEF2("SeparatorText", 1, js_imgui_SeparatorText),
        JS_CFUNC_DEF2("SameLine", 0, js_imgui_SameLine),
        JS_CFUNC_DEF2("Spacing", 0, js_imgui_Spacing),
        JS_CFUNC_DEF2("SetCursorPos", 1, js_imgui_SetCursorPos),
        JS_CFUNC_DEF2("IsItemFocused", 0,js_imgui_IsItemFocused),
        JS_CFUNC_DEF2("IsItemHovered", 0,js_imgui_IsItemHovered),
        JS_CFUNC_DEF2("IsItemActive", 0,js_imgui_IsItemActive),
        JS_CFUNC_DEF2("IsItemEdited", 0,js_imgui_IsItemEdited),
        JS_CFUNC_DEF2("IsItemActivated", 0,js_imgui_IsItemActivated),
        JS_CFUNC_DEF2("IsItemDeactivated", 0,js_imgui_IsItemDeactivated),
        JS_CFUNC_DEF2("IsItemDeactivatedAfterEdit", 0,js_imgui_IsItemDeactivatedAfterEdit),
        JS_CFUNC_DEF2("IsItemVisible", 0,js_imgui_IsItemVisible),
        JS_CFUNC_DEF2("IsItemClicked", 0,js_imgui_IsItemClicked),
        JS_CFUNC_DEF2("IsItemToggledOpen", 0,js_imgui_IsItemToggledOpen),
        JS_CFUNC_DEF2("GetItemRectMin", 0,js_imgui_GetItemRectMin),
        JS_CFUNC_DEF2("GetItemRectMax", 0,js_imgui_GetItemRectMax),
        JS_CFUNC_DEF2("GetItemRectSize", 0,js_imgui_GetItemRectSize),
        JS_CFUNC_DEF2("SetClipboardText", 1, js_imgui_SetClipboardText),
        JS_CFUNC_DEF2("GetClipboardText", 0, js_imgui_GetClipboardText),
        JS_CFUNC_DEF2("GetForegroundDrawList", 0, js_imgui_GetForegroundDrawList),
        JS_CFUNC_DEF2("GetIO", 0, js_imgui_GetIO),
};

static int js_imgui_init(JSContext* ctx, JSModuleDef* m) {
    imguiclass_init();

    return JS_SetModuleExportList(ctx, m, js_imgui_funcs, _countof(js_imgui_funcs));
}

JSModuleDef* js_init_module_imgui(JSContext* ctx, const char* module_name) {
    JSModuleDef* m = JS_NewCModule(ctx, module_name, js_imgui_init);
    if(!m)
        return NULL;
    JS_AddModuleExportList(ctx, m, js_imgui_funcs, _countof(js_imgui_funcs));
    return m;
}