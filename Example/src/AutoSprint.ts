//import * as 工具 from '工具'
import * as 事件系统 from '事件系统'
import SignCode from 'SignCode'
import * as http from 'http'


(function(模块 : 类型_公共模块){
    const logger = new spdlog(模块.name);

    事件系统.监听事件("onSave",function(data : any){
        data[模块.name] = {
            enable: 模块.enable
        };
        return true;
    });

    事件系统.监听事件("onLoad",function(data : any){
        let thisData = data[模块.name];
        if(thisData){
            模块.enable = thisData.enable;
        }
    });

    const findTopCall = (ptr:number)=>{
        for(let i = 0; i < 500; i++){
            if(new NativePoint(ptr - i).getuchar() == 0xCC){
                return ptr - i + 1;
            }
        }
        return 0;
    }

    let sign_old = new SignCode("移动输入特征码old", true);
    sign_old.AddSign("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 ? ? 49");
    sign_old.AddSign("41 88 01 48 8B ? 0F", findTopCall);
    sign_old.AddSign("41 88 ? 01 0F B6 ? ? 41 88 ? 02 0F B6 ? ? 41 88 ? 03 0F B6 ? ?  41 88 ? 04", findTopCall);
    if(sign_old.isOK()){
        let offset : number = new NativePoint(sign_old.get()).offset(18).getchar();
        // 创建hook
        let 输入Hook : HookBase = new HookBase(function(thi, ControlKey : NativePoint, a3, a4, a5, a6, a7, a8, a9, a10, a11 : Number){
            if(模块.enable){
                ControlKey.offset(offset).offset(8).setbool(true);
            }
            return 输入Hook.origin.call(thi,ControlKey,a3,a4,a5,a6,a7,a8,a9,a10,a11);
        },sign_old.get(), [NativeTypes.UnsignedLongLong, 
            NativeTypes.UnsignedLongLong/**This */, 
            NativeTypes.Pointer/*输入事件指针*/, 
            NativeTypes.UnsignedLongLong, 
            NativeTypes.UnsignedLongLong, 
            NativeTypes.UnsignedLongLong, 
            NativeTypes.UnsignedLongLong, 
            NativeTypes.UnsignedLongLong, 
            NativeTypes.UnsignedLongLong, 
            NativeTypes.UnsignedLongLong, 
            NativeTypes.UnsignedLongLong, 
            NativeTypes.UnsignedLongLong]);
        输入Hook.hook();
        logger.info("已成功加载模块:", 模块.name);
        return;
    }else{
        logger.warn("旧版函数_特征码未找到移动输入事件指针");
    }

    //////////////////////////////////////////////////////////////////////// 不同的版本使用第二套
    let sign_new = new SignCode("移动输入特征码new", true);
    sign_new.AddSign("0F B6 ? 88 ? 0F B6 42 01 88 41 01 0F");
    if(sign_new.isOK()){
        let 输入Hook : HookBase = new HookBase(function(thi, ControlKey : NativePoint){
            if(模块.enable){
                ControlKey.offset(8).setbool(true);
            }
            return 输入Hook.origin.call(thi,ControlKey);
        },sign_new.get(), [NativeTypes.UnsignedLongLong, 
            NativeTypes.UnsignedLongLong/**This */, 
            NativeTypes.Pointer/*输入事件指针*/]);
        输入Hook.hook();
    }else{
        logger.warn("新版函数_特征码未找到移动输入事件指针");
    }
    logger.info("模块已成功加载");
})(__模块__)