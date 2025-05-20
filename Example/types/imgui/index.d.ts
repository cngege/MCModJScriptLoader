declare module "imgui"{
    export function ShowDemoWindow(): void;
    export function ShowDemoWindow(open: ref_bool): void;
    export function Begin(name: string): boolean;
    export function Begin(name: string, open: ref_bool): boolean;
    export function Begin(name: string, open: ref_bool, flags: number): boolean;
    export function End(): void;
    export function Text(text: string): void;
    export function Button(label: string): boolean;
    export function Button(label: string, size: vec_2): boolean;
    export function Checkbox(label: string, checked: ref_bool): void;
    export function Toggle(label: string, checked: ref_bool): void;
    export function InputInt(label: string, value: ref_i, step?: number, step_fast?: number, flags?: number): Boolean;
    export function InputFloat(label: string, value: ref_f, step?: number, step_fast?: number, format?:string, flag?:number): Boolean;
    export function SliderInt(label: string, value: ref_i, minValue?: number, maxValue?: number, format?:string, flag?:number): Boolean;
    export function GetForegroundDrawList(): any;
    export function GetIO(): ImGuiIO;
    export class ImGuiIO{
        WantCaptureMouseUnlessPopupClose : boolean;
    }

    export class ImGuiForegroundDrawList{
        AddLine(p1: vec_2, p2: vec_2, color: number, thickness?: number): void;
    }
}

/**
 *         JS_CFUNC_DEF2("ShowDemoWindow", 0, js_imguiDemoWindow),
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
 * 
 */