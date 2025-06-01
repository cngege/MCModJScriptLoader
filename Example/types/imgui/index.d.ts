declare module "imgui"{
    export function ShowDemoWindow(): void;
    export function ShowDemoWindow(open: (v?:boolean)=>boolean): void;
    export function Begin(name: string): boolean;
    export function Begin(name: string, open: (v?:boolean)=>boolean): boolean;
    export function Begin(name: string, open: (v?:boolean)=>boolean, flags: number): boolean;
    export function End(): void;
    export function Text(...text: any[]): void;
    export function Button(label: string): boolean;
    export function Button(label: string, size: vec_2): boolean;
    export function Checkbox(label: string, checked: (v?:boolean)=>boolean): void;
    export function Toggle(label: string, checked: (v?:boolean)=>boolean): void;
    export function InputInt(label: string, value: (v?:number)=>number, step?: number, step_fast?: number, flags?: number): Boolean;
    export function InputFloat(label: string, value: (v?:number)=>number, step?: number, step_fast?: number, format?:string, flag?:number): Boolean;
    export function SliderInt(label: string, value: (v?:number)=>number, minValue: number, maxValue: number, format?:string, flag?:number): Boolean;
    export function SliderFloat(label: string, value: (v?:number)=>number, minValue: number, maxValue: number, format?:string, flag?:number): Boolean;
    export function Combo(label: string, value: (v?:number)=>number, list: string[], popup_max_height?:number): Boolean;
    export function ColorEdit3(label: string, color: color_3, flags?: number): Boolean;
    export function RadioButton(label: string, v: boolean): Boolean;
    export function RadioButton(label: string, value: (v?:number)=>number, v_button: number): Boolean;
    /**
     * 进度条
     * @param fraction [0.0, 1.0] 进度条的值 
     * @param size_arg 进度条尺寸： - x: 宽度（负数 = 自动填充父窗口宽度） - y: 高度（0 = 默认高度）
     * @param overlay 覆盖在进度条上的文本
     */
    export function ProgressBar(fraction: number, size_arg?: vec_2, overlay?: string): void;
    /**
     * 列出来的多选列表框
     * @param label 提示文本
     * @param current_item callback 函数，返回当前选中的索引
     * @param items 下拉列表的选项数组
     * @param height_in_items 
     */
    export function ListBox(label: string, current_item: (v?:number)=>number, items: string[], height_in_items?: number): Boolean;
    /**
     * 类似于没点的ul->li列表 可以控制选中效果，像按钮一样的点击效果
     * @param label 提示文本
     * @param selected callback 函数，返回当前选中状态
     * @param flags 
     * @param size 
     */
    export function Selectable(label: string, selected: (v?:boolean)=>boolean, flags?: number, size?: vec_2): Boolean;
    /**
     * 类似于没点的ul->li列表 可以控制选中效果，像按钮一样的点击效果
     * @param label 提示文本
     * @param selected 该项是否被选中
     * @param flags 
     * @param size 
     */
    export function Selectable(label: string, selected?: boolean, flags?: number, size?: vec_2): Boolean;
    export function InputText(label: string,size: number, buf: (v?:string)=>string, flags?: number): Boolean;
    export function TreeNode(label: string, flags?: number): Boolean;
    export function TreePop():void;
    /**
     * 折叠头
     * @param label 
     * @param flags 
     */
    export function CollapsingHeader(label: string, flags?: number): Boolean;
    /**
     * 画一条横线
     */
    export function Separator():void;
    export function SeparatorText(text: string):void;
    export function SameLine(offset_from_start_x?:number, spacing?:number):void;
    export function Spacing():void;
    export function SetCursorPos(pos:vec_2):void;
    export function IsItemFocused():boolean;
    export function IsItemHovered():boolean;
    export function IsItemActive():boolean;
    export function IsItemEdited():boolean;
    export function IsItemActivated():boolean;
    export function IsItemDeactivated():boolean;
    export function IsItemDeactivatedAfterEdit():boolean;
    export function IsItemVisible():boolean;
    export function IsItemClicked(mouse_button?:number):boolean;
    export function IsItemToggledOpen():boolean;
    export function GetItemRectMin():vec_2;
    export function GetItemRectMax():vec_2;
    export function GetItemRectSize():vec_2;
    export function SetClipboardText(text:string):void;
    export function GetClipboardText():string;
    /**
    ImGuiMouseCursor_None = -1,
    ImGuiMouseCursor_Arrow = 0,
    ImGuiMouseCursor_TextInput,         // When hovering over InputText, etc.
    ImGuiMouseCursor_ResizeAll,         // (Unused by Dear ImGui functions)
    ImGuiMouseCursor_ResizeNS,          // When hovering over a horizontal border
    ImGuiMouseCursor_ResizeEW,          // When hovering over a vertical border or a column
    ImGuiMouseCursor_ResizeNESW,        // When hovering over the bottom-left corner of a window
    ImGuiMouseCursor_ResizeNWSE,        // When hovering over the bottom-right corner of a window
    ImGuiMouseCursor_Hand,              // (Unused by Dear ImGui functions. Use for e.g. hyperlinks)
    ImGuiMouseCursor_Wait,              // When waiting for something to process/load.
    ImGuiMouseCursor_Progress,          // When waiting for something to process/load, but application is still interactive.
    ImGuiMouseCursor_NotAllowed,        // When hovering something with disallowed interaction. Usually a crossed circle.
    ImGuiMouseCursor_COUNT
     */
    export function GetMouseCursor():number;
    export function GetForegroundDrawList(): any;
    export function GetIO(): ImGuiIO;
    export class ImGuiIO{
        WantCaptureMouseUnlessPopupClose : boolean;
    }

    export class ImGuiForegroundDrawList{
        AddLine(p1: vec_2, p2: vec_2, color: number, thickness?: number): void;
        AddCircle(center: vec_2, radius: number, color: number, num_segments?: number, thickness?: number): void;
        AddText(pos: vec_2, color: number, text_begin: string,text_end?:string): void;
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