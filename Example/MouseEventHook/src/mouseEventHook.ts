//import * as 工具 from '工具'
import * as 事件系统 from '事件系统'
import SignCode from 'SignCode'
import * as http from 'http'
import * as ImGui from 'imgui'

(function(模块 : 类型_公共模块){
    const logger = new spdlog();
    let sign = new SignCode("鼠标事件指针", true);
    sign.AddSign("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 44 0F");
    sign.AddSignCall("E8 ? ? ? ? 40 B7 01 48 85 DB 74 ? 48");
    if(!sign.isOK()){
        logger.warn("特征码未找到鼠标事件指针");
        return;
    }

    // 创建一个关于鼠标事件的hook
    let 鼠标事件 : HookBase = new HookBase(function(thi : number, btn : number, isdown : number,x : number,y : number,rx : number,ry : number,a7 : number){
        // let 鼠标事件是否执行 : boolean = true;
        // // 事件广播 - 将鼠标事件广播出去
        // 事件系统.事件广播("onMouseHandle",function(ret : boolean){
        //     // 如果监听的事件中 有一个的返回值为false
        //     if(!ret) 鼠标事件是否执行 = false;
        // },btn, isdown, x ,y);
        // // 如果广播不要求拦截 则将鼠标事件传递回去
        // if(鼠标事件是否执行) {
        //     鼠标事件.origin.call(thi,btn,isdown,x,y,rx,ry,a7);
        // }
        let io = ImGui.GetIO();
        if(!io.WantCaptureMouseUnlessPopupClose){
            鼠标事件.origin.call(thi,btn,isdown,x,y,rx,ry,a7);
        }
    },sign.get(), [NativeTypes.Void/*返回值*/,NativeTypes.UnsignedLongLong/**this */,NativeTypes.Char/*鼠标按键*/,NativeTypes.Char/**是否按下 */,NativeTypes.Short/**鼠标X */,NativeTypes.Short/**鼠标Y */,NativeTypes.Short/**relativeMovementX */,NativeTypes.Short/**relativeMovementY */,NativeTypes.Char]);
    鼠标事件.hook();
    // http.Ajax({
    //     url : "http://stream1.shopch.jp/HLS/out1/prog_index.m3u8",
    //     success : function(res){
    //         logger.info("请求成功", JSON.stringify(res));
    //     },
    //     download : function(len : number, total : number){
    //         logger.info("下载进度", len, total);
    //         return true;
    //     }
    // });
    logger.info("已成功加载模块:", 模块.name);
})(__模块__)