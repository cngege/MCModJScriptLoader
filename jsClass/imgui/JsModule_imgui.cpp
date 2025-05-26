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


    //if(argc >= 1) {
    //    auto str = JSTool::toString(argv[0]);
    //    if(!str) return JS_ThrowTypeError(ctx, "参数1应传入字符串");
    //    bool ret = false;

    //    // 第二个参数
    //    if(argc >= 2) {
    //        auto ret_str = JSTool::ReferenceBoolCall(argv[0], [&](bool* v) {
    //            if(argc >= 3) {
    //                auto intval = JSTool::toInt(argv[2]);
    //                if(!intval) throw std::runtime_error("参数3需要一个Number 作为flag");
    //                ret = ImGui::Begin(str->c_str(), v, *intval);
    //            }
    //            ret = ImGui::Begin(str->c_str(), v);
    //        });
    //        if(!ret_str.empty()) {
    //            return JS_ThrowTypeError(ctx, ret_str.c_str());
    //        }
    //        return JS_NewBool(ctx, ret);
    //    }
    //    ret = ImGui::Begin(str->c_str());
    //    return JS_NewBool(ctx, ret);
    //}
    //return JS_ThrowTypeError(ctx, "至少需要一个参数,类型为String");
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

    //if(argc >= 1) {
    //    auto str = JSTool::toString(argv[0]);
    //    if(!str) return JS_ThrowTypeError(ctx, "参数1应传入字符串");
    //    bool ret = false;

    //    if(argc >= 2) {
    //        auto getxy = JSTool::getPropXY(argv[1]);
    //        if(!getxy) {
    //            return JS_ThrowTypeError(ctx, "参数2应包含属性x(Number),y(Number)");
    //        }
    //        ret = ImGui::Button(str->c_str(),{ getxy->at(0), getxy->at(1)});
    //        return JS_NewBool(ctx, ret);
    //    }
    //    ret = ImGui::Button(str->c_str());
    //    return JS_NewBool(ctx, ret);
    //}
    //return JS_ThrowTypeError(ctx, "至少需要一个参数,类型为String");
}

static JSValue js_imgui_Checkbox(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    //1   字符串 标题
    //[2] bool指针 - 引用值|bool ischeck

    bool ret = false;
    std::optional<std::string> label;
    //v

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

    //if(argc >= 2) {
    //    auto str = JSTool::toString(argv[0]);
    //    if(!str) return JS_ThrowTypeError(ctx, "参数1应传入字符串");
    //    bool ret = false;

    //    auto ret_str = JSTool::ReferenceBoolCall(argv[1], [&](bool* v) {
    //        ret = ImGui::Checkbox(str->c_str(), v);
    //    });
    //    if(!ret_str.empty()) {
    //        return JS_ThrowTypeError(ctx, ret_str.c_str());
    //    }
    //    return JS_NewBool(ctx, ret);

    //    //bool open = false;
    //    //JSValue referr = JSTool::ReferenceValue(&open, argv[1]);
    //    //if(!JS_IsUndefined(referr)) return referr;

    //    //ret = ImGui::Checkbox(str->c_str(), &open);
    //    //JSTool::ReferenceValue(open, argv[1]);
    //    //return JS_NewBool(ctx, ret);
    //}
    //return JS_ThrowTypeError(ctx, "至少需要2个参数,类型为[String, bool引用对象]");
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

    //if(argc >= 2) {
    //    auto str = JSTool::toString(argv[0]);
    //    if(!str) return JS_ThrowTypeError(ctx, "参数1应传入字符串");
    //    bool ret = false;


    //    auto ret_str = JSTool::ReferenceBoolCall(argv[1], [&](bool* v) {
    //        if(argc >= 3) {
    //            auto v3 = JSTool::getPropXY(argv[2]);
    //            if(!v3) {
    //                return JS_ThrowTypeError(ctx, "参数3应传入对象{x,y}");
    //            }
    //            ImVec2 size = { v3->at(0),v3->at(1) };
    //            ret = ImGui::Toggle(str->c_str(), v, size);
    //        }
    //        else {
    //            ret = ImGui::Toggle(str->c_str(), v);
    //        }
    //    });
    //    if(!ret_str.empty()) {
    //        return JS_ThrowTypeError(ctx, ret_str.c_str());
    //    }
    //    return JS_NewBool(ctx, ret);

    //    //bool open = false;
    //    //JSValue referr = JSTool::ReferenceValue(&open, argv[1]);
    //    //if(!JS_IsUndefined(referr)) return referr;

    //    //if(argc >= 3) {
    //    //    // 解析 xy
    //    //    auto v = JSTool::getPropXY(argv[2]);
    //    //    if(!v) {
    //    //        return JS_ThrowTypeError(ctx, "参数3应传入对象{x,y}");
    //    //    }
    //    //    ImVec2 size = { v->at(0),v->at(1) };
    //    //    ret = ImGui::Toggle(str->c_str(), &open, size);
    //    //}
    //    //else {
    //    //    ret = ImGui::Toggle(str->c_str(), &open);
    //    //}
    //    //if(ret) JSTool::ReferenceValue(open, argv[1]);
    //    //return JS_NewBool(ctx, ret);
    //}
    //return JS_ThrowTypeError(ctx, "至少需要2个参数,类型为[String, bool引用对象]");
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
    std::optional<float> step = 1;
    std::optional<float> step_fast = 100;
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

    //if(argc < 2) return JS_ThrowTypeError(ctx, "至少需要2个参数,类型为[String, v引用对象]");
    //bool ret = false;
    //std::string label;
    //float v = 0.f;
    //float step = 0.f;
    //float step_fast = 0.f;
    //std::string format = "%.3f";
    //int flags = 0;

    //auto str = JSTool::toString(argv[0]);
    //if(!str) return JS_ThrowTypeError(ctx, "参数1应传入字符串");
    //label = *str;

    //JSValue referr = JSTool::ReferenceValue(&v, argv[1]);
    //if(!JS_IsUndefined(referr)) return referr;

    //if(argc >= 3) {
    //    auto referr2 = JSTool::toFloat(argv[2]);
    //    if(!referr2) return JS_ThrowTypeError(ctx, "参数3应传入float[step]");
    //    step = *referr2;

    //    if(argc >= 4) {
    //        auto referr3 = JSTool::toFloat(argv[3]);
    //        if(!referr3) return JS_ThrowTypeError(ctx, "参数4应传入float[step_fast]");
    //        step_fast = *referr3;

    //        if(argc >= 5) {
    //            auto strfmt = JSTool::toString(argv[4]);
    //            if(!strfmt) return JS_ThrowTypeError(ctx, "参数5应传入字符串");
    //            format = *str;

    //            if(argc >= 5) {
    //                auto referr5 = JSTool::toInt(argv[5]);
    //                if(!referr5) return JS_ThrowTypeError(ctx, "参数6应传入int[flag]");
    //                flags = *referr5;
    //            }
    //        }
    //    }
    //}
    //ret = ImGui::InputFloat(label.c_str(), &v, step, step_fast, format.c_str(), flags);
    //JSTool::ReferenceValue(v, argv[1]);
    //return JS_NewBool(ctx, ret);
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

    //if(argc < 4) return JS_ThrowTypeError(ctx, "至少需要4个参数,类型为[String, v引用对象, int_min, int_max]");
    //bool ret = false;
    //std::string label;
    //int v = 0;
    //int min = -100;
    //int max = 100;
    //std::string format = "%d";
    //int flags = 0;

    //auto str = JSTool::toString(argv[0]);
    //if(!str) return JS_ThrowTypeError(ctx, "参数1应传入字符串");
    //label = *str;

    //JSValue referr = JSTool::ReferenceValue(&v, argv[1]);
    //if(!JS_IsUndefined(referr)) return referr;

    //if(argc >= 3) {
    //    auto referr2 = JSTool::toInt(argv[2]);
    //    if(!referr2) return JS_ThrowTypeError(ctx, "参数3应传入int[minvalue]");
    //    min = *referr2;

    //    if(argc >= 4) {
    //        auto referr3 = JSTool::toInt(argv[3]);
    //        if(!referr3) return JS_ThrowTypeError(ctx, "参数4应传入int[maxvalue]");
    //        max = *referr3;

    //        if(argc >= 5) {
    //            auto str = JSTool::toString(argv[4]);
    //            if(!str) return JS_ThrowTypeError(ctx, "参数5应传入字符串");
    //            format = *str;

    //            if(argc >= 5) {
    //                auto referr5 = JSTool::toInt(argv[5]);
    //                if(!referr5) return JS_ThrowTypeError(ctx, "参数6应传入int[flag]");
    //                flags = *referr5;
    //            }
    //        }
    //    }
    //}
    //ret = ImGui::SliderInt(label.c_str(), &v, min, max, format.c_str(), flags);
    //JSTool::ReferenceValue(v, argv[1]);
    //return JS_NewBool(ctx, ret);
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