#include "imguiclass.h"
#include "imgui/imgui.h"
#include "spdlog/spdlog.h"

namespace {
    static JSClassID _ImGuiIO_id;
    static JSClassDef _ImGuiIO_classevent = {
        .class_name{"ImGuiIO"},
        .finalizer{[](JSRuntime* rt, JSValue val) {}
        }
    };

    static JSClassID _ForegroundDrawList_id;
    static JSClassDef _ForegroundDrawList_classevent = {
        .class_name{"ForegroundDrawList"},
        .finalizer{[](JSRuntime* rt, JSValue val) {}
        }
    };
}

JSClassID JSImGuiIO::getImGuiIOID() {
    return _ImGuiIO_id;
}

JSClassID JSForegroundDrawList::getForegroundDrawListID() {
    return _ForegroundDrawList_id;
}

static JSValue js_imgui__ForegroundDrawList_AddLine(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    //if(argc < 3) return JS_ThrowTypeError(ctx, "至少需要3个参数,类型为[vec2, vec2, color(0xRRGGBBAA), float=1.f]");
    ////this : ImDrawList*

    std::optional<ImVec2> start;
    std::optional<ImVec2> end;
    std::optional<int> color;
    std::optional<float> thickness = 1.0f;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(start)
        .Parse(end)
        .Parse(color)
        .Parse(thickness)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    ImDrawList* draw = (ImDrawList*)JS_GetOpaque(this_val, _ForegroundDrawList_id);
    draw->AddLine(*start, *end, *color, *thickness);
    return JS_UNDEFINED;

    //auto XY1 = JSTool::getPropXY(argv[0]);
    //if(!XY1) return JS_ThrowTypeError(ctx, "参数1需要属性x,y");

    //auto XY2 = JSTool::getPropXY(argv[1]);
    //if(!XY2) return JS_ThrowTypeError(ctx, "参数2需要属性x,y");

    //auto color = JSTool::toInt(argv[2]);
    //if(!color) return JS_ThrowTypeError(ctx, "参数3需要Number");

    //float thickness = 1.f;
    //if(argc >= 4) {
    //    auto _thickness = JSTool::toFloat(argv[3]);
    //    if(!_thickness) return JS_ThrowTypeError(ctx, "参数4需要Float");
    //    if(_thickness) thickness = *_thickness;
    //}
    //
    //ImDrawList* draw = (ImDrawList*)JS_GetOpaque(this_val, _ForegroundDrawList_id);
    //draw->AddLine({ XY1->at(0), XY1->at(1) }, { XY2->at(0), XY2->at(1) }, *color, thickness);
    //return JS_UNDEFINED;
}

static JSValue js_imgui__ForegroundDrawList_AddCircle(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    //if(argc < 3) return JS_ThrowTypeError(ctx, "至少需要3个参数,类型为[vec2, float, color(0xRRGGBBAA), int num_segments , float=1.f]");
    std::optional<ImVec2> pos;
    std::optional<float> radius;
    std::optional<int> color;
    std::optional<int> num_segments = 0;
    std::optional<float> thickness = 1.0f;

    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(pos)
        .Parse(radius)
        .Parse(color)
        .Parse(num_segments)
        .Parse(thickness)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }

    ImDrawList* draw = (ImDrawList*)JS_GetOpaque(this_val, _ForegroundDrawList_id);
    draw->AddCircle(*pos, *radius, *color, *num_segments, *thickness);
    return JS_UNDEFINED;

    //if(argc < 3) return JS_ThrowTypeError(ctx, "至少需要3个参数,类型为[vec2, float, color(0xRRGGBBAA), int num_segments , float=1.f]");

    //auto XY1 = JSTool::getPropXY(argv[0]);
    //if(!XY1) return JS_ThrowTypeError(ctx, "参数1需要属性x,y");

    //auto F2 = JSTool::toFloat(argv[1]);
    //if(!F2) return JS_ThrowTypeError(ctx, "参数2需要属性float");

    //auto I3 = JSTool::toInt(argv[2]);
    //if(!I3) return JS_ThrowTypeError(ctx, "参数3需要属性int");

    //int I4_v = 0;
    //float F5_v = 1.0f;
    //if(argc >= 4) {
    //    auto _I4 = JSTool::toInt(argv[3]);
    //    if(!_I4) return JS_ThrowTypeError(ctx, "参数4需要属性int");
    //    I4_v = *_I4;

    //    if(argc >= 5) {
    //        auto _F5 = JSTool::toFloat(argv[4]);
    //        if(!_F5) return JS_ThrowTypeError(ctx, "参数5需要属性float");
    //        F5_v = *_F5;
    //    }
    //}
    //ImDrawList* draw = (ImDrawList*)JS_GetOpaque(this_val, _ForegroundDrawList_id);
    //draw->AddCircle({ XY1->at(0), XY1->at(1) }, F2.value(), I3.value(), I4_v, F5_v);
    //return JS_UNDEFINED;

}

static JSValue js_imgui__ForegroundDrawList_AddText(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {

    std::optional<ImVec2> pos;
    std::optional<int> color;
    std::optional<std::string> text_begin;
    std::optional<std::string> text_end = "0";
    auto retV = JSTool::createParseParameter(argc, argv)
        .Parse(pos)
        .Parse(color)
        .Parse(text_begin)
        .Parse(text_end)
        .Build();
    if(!retV.empty()) {
        return JS_ThrowTypeError(ctx, retV.c_str());
    }
    ImDrawList* draw = (ImDrawList*)JS_GetOpaque(this_val, _ForegroundDrawList_id);
    draw->AddText(pos.value(), ImU32(color.value()), text_begin->c_str(), (*text_end == "0") ? nullptr : text_end.value().c_str());
    return JS_UNDEFINED;
}


static JSValue js_imgui__IO_WantCaptureMouseUnlessPopupClose(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    ImGuiIO* io = (ImGuiIO*)JS_GetOpaque(this_val, _ImGuiIO_id);
    return JS_NewBool(ctx, io->WantCaptureMouseUnlessPopupClose);
}








void imguiclass_init() {
    JSContext* ctx = JSManager::getInstance()->getctx();
    auto rt = JS_GetRuntime(ctx);
    /// ImGuiIO ///
    {
        JS_NewClassID(rt, &_ImGuiIO_id);
        JS_NewClass(rt, _ImGuiIO_id, &_ImGuiIO_classevent);
        JSValue proto_imguio = JS_NewObject(ctx);
        {
            //JS_SetPropertyStr(ctx, proto_imguio, "WantCaptureMouseUnlessPopupClose", JS_NewCFunction(ctx, js_imgui__ForegroundDrawList_AddLine, "WantCaptureMouseUnlessPopupClose", 0));
            auto getter = JS_NewCFunction(ctx, js_imgui__IO_WantCaptureMouseUnlessPopupClose, "get_WantCaptureMouseUnlessPopupClose", 0);
            JS_DefinePropertyGetSet(ctx, proto_imguio, JS_NewAtom(ctx, "WantCaptureMouseUnlessPopupClose"), getter, JS_UNDEFINED, JS_PROP_CONFIGURABLE);
        }

        JS_SetClassProto(ctx, _ImGuiIO_id, proto_imguio);
    }

    /// ForegroundDrawList ///
    {
        JS_NewClassID(rt, &_ForegroundDrawList_id);
        JS_NewClass(rt, _ForegroundDrawList_id, &_ForegroundDrawList_classevent);

        JSValue protoInstance = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, protoInstance, "AddLine", JS_NewCFunction(ctx, js_imgui__ForegroundDrawList_AddLine, "AddLine", 3));
        JS_SetPropertyStr(ctx, protoInstance, "AddCircle", JS_NewCFunction(ctx, js_imgui__ForegroundDrawList_AddCircle, "AddCircle", 3));
        JS_SetPropertyStr(ctx, protoInstance, "AddText", JS_NewCFunction(ctx, js_imgui__ForegroundDrawList_AddText, "AddText", 3));

        JS_SetClassProto(ctx, _ForegroundDrawList_id, protoInstance);
    }
}