import * as 工具 from '工具'
import * as 事件系统 from '事件系统'
import SignCode from 'SignCode'
import * as http from 'http'
import * as ImGui from 'imgui'

interface 模块数据{
    dropCount: number;
    printDebug: boolean;
    拦截事件: boolean;
}

(function(模块 : 类型_公共模块){
    const logger = new spdlog(模块.name);
    const otherdata : 模块数据 = {
        dropCount: 100,
        printDebug: false,
        拦截事件: false,
    }

    事件系统.监听事件("onSave",function(data : any){
        data[模块.name] = {
            enable: 模块.enable,
            dropCount: otherdata.dropCount
        };
        return true;
    });

    事件系统.监听事件("onLoad",function(data : any){
        let thisData = data[模块.name];
        if(thisData){
            模块.enable = thisData.enable;
            otherdata.dropCount = thisData.dropCount;
        }
    });

    //logger.setLevel(spdlog.Level.Debug);
    const findTopCall = (ptr:number)=>{
        for(let i = 0; i < 908; i++){   // 908 字节, 这函数差不多这么长
            if(new NativePoint(ptr - i).getuchar() == 0xCC){
                return ptr - i + 1;
            }
        }
        return 0;
    }

    let sign_玩家破坏方块 = new SignCode("玩家破坏方块特征码", true, true);
    sign_玩家破坏方块.AddSign("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 4D 8B E8 4C 8B F2 4C 8B F9");
    sign_玩家破坏方块.AddSignCall("E8 ? ? ? ? E9 ? ? ? ? 49 8B CE E8 ? ? ? ? 48 85");
    sign_玩家破坏方块.AddSignCall("75 ? 4D 8B ? 48 8B 57 ? 49 8B CE E8", 13);

    sign_玩家破坏方块.AddSign("48 8B ? ? 48 85 ? 0F 84 ? ? ? ? ? ? 40 38 B7", findTopCall);
    sign_玩家破坏方块.AddSign("75 ? 48 8D 8F ? ? ? ? E8 ? ? ? ? 84 ? 74 ? 48 8B 9F ? ? ? ? 48 8B BF ? ? ? ? 48 3B ? 74 ? 66 ? 48 8B", findTopCall);
    sign_玩家破坏方块.AddSign("48 85 ? 74 ? F2 41 0F 10 45 00 F2 0F 11 45 ? 41", findTopCall);
    sign_玩家破坏方块.AddSign("41 8B 45 ? 89 45 ? 48 8D ? ? ? ? ? 48 89 45 ? 4C", findTopCall);
    sign_玩家破坏方块.AddSign("4C 89 75 ? 4C 89 7D ? 48 8B ? 48", findTopCall);
    sign_玩家破坏方块.AddSign("48 8D 55 ? 48 8B 40 ? FF 15 ? ? ? ? 90 49 8B 8E", findTopCall);
    sign_玩家破坏方块.AddSign("48 8B 80 ? ? ? ? FF 15 ? ? ? ? 84 ? 0F 85 ? ? ? ? 49 8B CE", findTopCall);
    sign_玩家破坏方块.AddSign("E8 ? ? ? ? 4C 8B ? 0F 57 ? 33 ? 0F 11 45 ? 48 89 45 ? 49 8B 96 ? ? ? ? 48", findTopCall);
    sign_玩家破坏方块.AddSign("48 85 ? 74 ? 48 8B ? 48 8B 81 ? ? ? ? 48 8B ? FF 15 ? ? ? ? 48 8B ? EB ? 48 8B ? E9", findTopCall);
    sign_玩家破坏方块.AddSign("65 48 8B ? ? ? ? ? ? 48 8B ? B8 ? ? ? ? 80 3C ? 00 75 ? E8 ? ? ? ? BF ? ? ? ? 48 ? ? 0F 57 ? F3 0F 7F 45 ? 48", findTopCall);
    sign_玩家破坏方块.AddSign("48 8D 47 ? 48 85 ? 48 0F 44 ? BE", findTopCall);
    sign_玩家破坏方块.AddSign("48 85 ? 74 ? F0 FF 41 ? 48 8B 48 ? 48 8B 40 ? 48 89 45 ? 48 8B 5D ? 48 89 4D ? 48 85 ? 74 ? 8B C6 F0 0F C1 ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? FF ? ? ? ? ? 8B C6 F0 0F C1 43 ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B 40 ? FF ? ? ? ? ? 48", findTopCall);
    sign_玩家破坏方块.AddSign("48 83 7D ? ? 74 ? 48 89 7D ? 49", findTopCall);
    sign_玩家破坏方块.AddSign("49 8B 96 ? ? ? ? 80 BA ? ? ? ? ? 75 ? 48 8B 8A ? ? ? ? 48 8B ? 8B 52 ? 48 8B 40 ? FF 15 ? ? ? ? 48 8B ? EB ? 48 8D 1D", findTopCall);
    sign_玩家破坏方块.AddSign("49 8B ? ? 48 8D ? ? 49 8B ? 48 8B ? ? ? ? ? FF ? ? ? ? ? 8B", findTopCall);
    sign_玩家破坏方块.AddSign("C7 45 ? 00 00 00 00 C7 45 ? ? ? ? ? 48 89 5D ? F2", findTopCall);
    sign_玩家破坏方块.AddSign("F2 41 0F 10 ? ? F2 0F 11 45 ?  41 8B 4D ? 89 4D ? 89 45 E3", findTopCall);
    sign_玩家破坏方块.AddSign("4C 89 65 ? 49 8B ? ? 49 8B ? 48 8B 80 ? ? ? ? FF ? ? ? ? ? 48 8B", findTopCall);
    sign_玩家破坏方块.AddSign("48 8B ? 48 8B ? 48 8B ? ? ? ? ? 48 8B ? FF ? ? ? ? ? 84 ? 75 ? 49 8B ? ? 48 85 ? 0F 84 ? ? ? ? 48 8D 45 ? 48 89 44 24 ? 48 8D 45 ? 48 89 44 24 ? 4D 8B ? 4D 8B ? 49 8B ? E8 ? ? ? ? 90 45 33", findTopCall);
    sign_玩家破坏方块.AddSign("E8 ? ? ? ? 90 45 33 ? 4C 89 7D ? 48 8B 7D ? 41 8B ? 48 89 5D ? 48 85 ? 74 ? 8B", findTopCall);
    sign_玩家破坏方块.AddSign("83 ? ? 75 ? 48 8B ? 48 8B ? 48 8B 40 ? FF 15 ? ? ? ? 49 8D 4E ? E8 ? ? ? ? 48", findTopCall);
    sign_玩家破坏方块.AddSign("74 ? F3 0F ? ? ? 49 8B ? E8 ? ? ? ? 48 8B ? ? 48 ? ? E8 ? ? ? ? 48",findTopCall);

    if(sign_玩家破坏方块.isOK()){
        let 玩家破坏方块Hook : HookBase = new HookBase(function(block : number, player : number, pos : number){
            if(otherdata.printDebug) {
                let gameBase = 工具.获取基址();
                logger.debug("玩家破坏方块Hook - Block:", block.toString(16),"- Player:", player.toString(16),"- Pos:", pos.toString(16));
                logger.debug("虚表偏移 Block:", (new NativePoint(block).getlong()-gameBase).toString(16),"- Player:", (new NativePoint(player).getlong()-gameBase).toString(16));
            }
            if(otherdata.拦截事件) return 0;
            if(模块.enable){
                for(let i = 0; i < otherdata.dropCount - 1; i++){
                    玩家破坏方块Hook.origin.call(block, player, pos);
                }
            }
            //////
            // vIndex: 21
            //virtual void teleportTo(::Vec3 const& pos, bool shouldStopRiding, int, int, bool keepVelocity);

            // let p = new NativePoint(player);
            // let call = p.getpoint().offset(21 * 8).getpoint();
            // call.setAgree([NativeTypes.Void,NativeTypes.UnsignedLongLong, NativeTypes.UnsignedLongLong,NativeTypes.Bool, NativeTypes.Int, NativeTypes.Int, NativeTypes.Bool]);
            // if(call.toNumber() % 10 == 0){
            //     call.call(player, pos, true, 0, 0, false);
            //     logger.debug("teleportTo", call.toNumber());
            // }



            //////
            return 玩家破坏方块Hook.origin.call(block, player, pos);
        },sign_玩家破坏方块.get(), [NativeTypes.UnsignedLongLong, 
            NativeTypes.UnsignedLongLong/**This Block */, 
            NativeTypes.UnsignedLongLong/**Player */, 
            NativeTypes.UnsignedLongLong/**pos */, 
        ]);
        玩家破坏方块Hook.hook();
    }else{
        logger.warn("玩家破坏方块特征码未找到");
        return;
    }
    事件系统.监听事件("onRender", () => {
        if(模块.enable){
            if(ImGui.Begin(模块.name)){
                ImGui.Text("掉落物倍数");
                ImGui.SliderInt("掉落物数量", (_ = otherdata.dropCount)=> otherdata.dropCount = _, 1, 200);
                if(ImGui.Button("重置掉落物数量")){
                    otherdata.dropCount = 100;
                }
                ImGui.Separator();
                ImGui.Checkbox("拦截事件", (_ = otherdata.拦截事件) => otherdata.拦截事件 = _);
                ImGui.Separator();
                ImGui.Checkbox("打印Debug信息", (_ = otherdata.printDebug) => otherdata.printDebug = _);
            }
            ImGui.End();
        }
    })

    logger.info("模块已成功加载");
})(__模块__)


