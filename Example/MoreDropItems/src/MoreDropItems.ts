//import * as 工具 from '工具'
import * as 事件系统 from '事件系统'
import SignCode from 'SignCode'
import * as http from 'http'

(function(模块 : 类型_公共模块){
    const logger = new spdlog();

    //logger.setLevel(spdlog.Level.Debug);
    const findTopCall = (ptr:number)=>{
        for(let i = 0; i < 300; i++){
            if(new NativePoint(ptr - i).getchar() == 0xCC){
                return ptr - i + 1;
            }
        }
        return 0;
    }

    let sign_玩家破坏方块 = new SignCode("玩家破坏方块特征码", true);
    sign_玩家破坏方块.AddSign("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 4D 8B E8 4C 8B F2 4C 8B F9");
    sign_玩家破坏方块.AddSignCall("E8 ? ? ? ? E9 ? ? ? ? 49 8B CE E8 ? ? ? ? 48 85");
    sign_玩家破坏方块.AddSignCall("75 ? 4D 8B ? 48 8B 57 ? 49 8B CE E8", 13);

    sign_玩家破坏方块.AddSignCall("48 8B 59 78 48 85 DB 0F 84 ? ? ? ? 80",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("80 BB ? ? ? ? 00 75 ? E8 ? ? ? ? 84 ? 74 ? 48 8B 83 ? ? ? ? 48 8B 93 ? ? ? ? 48 3B ? 74 ? 48 8B 08 80 79 ? ? 74 ? 48 83 C0 ? 48 3B C2 75 ? EB",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("48 85 ? 74 ? F2 41 0F 10 45 00 F2 0F 11 45 ? 41",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("41 8B 45 ? 89 45 ? 48 8D ? ? ? ? ? 48 89 45 ? 4C",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("4C 89 75 ? 48 89 7D ? 48 8B ? 48",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("48 8D 55 ? 48 8B 40 ? FF 15 ? ? ? ? 90 49 8B 8E",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("48 8B 80 ? ? ? ? FF 15 ? ? ? ? 84 ? 0F 85 ? ? ? ? 49 8B CE",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("E8 ? ? ? ? 4C 8B ? 0F 57 ? 33 ? 0F 11 45 ? 48 89 45 ? 49",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("49 8B CE E8 ? ? ? ? 4C 8B ? 0F 57 ? F3 0F 7F 45 ? 33",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("48 89 4D ? 48 83 C0 ? 4D 85 ? 48 0F 44 ? 8D",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("8D 71 ? 48 85 ? 74 ? 48 8B 48",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("48 85 ? 74 ? F0 FF 41 ? 48 8B 48 ? 48 8B 40 ? 48 89 45 ? 48 8B 5D ? 48 89 4D ? 48 85 ? 74 ? 8B C6 F0 0F C1 ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? FF ? ? ? ? ? 8B C6 F0 0F C1 43 ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B 40 ? FF ? ? ? ? ? 48",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("74 ? 4C 89 7D ? 49",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("49 8B 96 ? ? ? ? 80 BA ? ? ? ? ? 75 ? 48 8B 8A ? ? ? ? 48 8B ? 8B 52 ? 48 8B 40 ? FF 15 ? ? ? ? 48 8B ? EB ? 48 8D 1D",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("49 8B ? ? 48 8D ? ? 49 8B ? 48 8B ? ? ? ? ? FF ? ? ? ? ? 8B",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("45 ? ? 44 89 7D ? C7 45 ? ? ? ? ? 48 89",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("F2 41 0F 10 ? ? F2 0F 11 45 ?  41 8B 4D ? 89 4D ? 89 45 E3",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("4C 89 65 ? 49 8B ? ? 49 8B ? 48 8B 80 ? ? ? ? FF ? ? ? ? ? 48 8B",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("48 8B ? 48 8B ? ? ? ? ? 48 8B ? FF ? ? ? ? ? 84 ? 75 ? 48 8B ? ? 48 85 ? 0F 84 ? ? ? ? 48 8D 45 ? 48 89 44 24 ? 48 8D 45 ? 48 89 44 24 ? 4C 8B ? 4D 8B ? 49",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("E8 ? ? ? ? 90 4C 89 7D ? 48 8B 7D ? 49 8B ? 48 89 5D ? 48 85 ? 74 ? 8B ? F0 0F C1 ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? FF ? ? ? ? ? 8B ? F0 0F C1 ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? ? FF 15 ? ? ? ? 48 8B 5D ? 4C 89 ? ? 48 85 ? 74 ? 8B ? F0 0F C1 ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? FF 15 ? ? ? ? F0 0F",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("83 ? ? 75 ? 48 8B ? 48 8B ? 48 8B 40 ? FF 15 ? ? ? ? 49 8D 4E ? E8 ? ? ? ? 48",0, findTopCall);
    sign_玩家破坏方块.AddSignCall("74 ? F3 0F 10 ? ? 49 8B ? E8 ? ? ? ? 48 8B 4D ? 48 33 ? E8",0, findTopCall);

    if(sign_玩家破坏方块.isOK()){
        let 玩家破坏方块Hook : HookBase = new HookBase(function(block : number, player : number, pos : number){
            if(模块.enable){
                for(let i = 0; i < 99; i++){
                    玩家破坏方块Hook.origin.call(block, player, pos);
                }
            }
            return 玩家破坏方块Hook.origin.call(block, player, pos);
        },sign_玩家破坏方块.get(), [NativeTypes.UnsignedLongLong, 
            NativeTypes.UnsignedLongLong/**This Block */, 
            NativeTypes.UnsignedLongLong/**Player */, 
            NativeTypes.UnsignedLongLong/**pos */, 
        ]);
        玩家破坏方块Hook.hook();
    }else{
        logger.warn("玩家破坏方块特征码未找到");
    }
    logger.info("已成功加载模块:", 模块.name);
})(__模块__)