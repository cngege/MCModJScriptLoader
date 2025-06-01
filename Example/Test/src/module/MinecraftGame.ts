
import SignCode from 'SignCode'
import * as 事件系统 from '事件系统'
import * as 工具 from '工具'

const logger = new spdlog();

// MinecraftGame 模块
export default class MinecraftGame{
    // MG 地址存在于 游戏基址中， 是可以直接通过基址定位到的
    // 如何获取相关特征码
    // 1. 首先通过CI拿到MG地址, CI构造函数的第二个参数就是
    // 2. 然后CE搜索这个地址，发现最下方有基址，看内存
    // 3. 这个地址上方大概 16 字节左右有地址一直在变化，通过定位这个变化的地址位置，定位MG地址
    // 4. 在这个变化的地址上 找写入，能看到几处代码直接显示了地址位置
    static #IsInit = false;
    static #基址: number = 0x0;
    
    static __init(){
        if(MinecraftGame.#IsInit == true) return true;
        MinecraftGame.#IsInit = true;
        // 要拿到这个基址偏移
        let sign = new SignCode("获取MinecraftGame的基址偏移", true, true, true);
        sign.AddSignCall("F2 0F 11 ? ? ? ? ? F2 0F 10 ? ? ? ? ? F2 0F 58 ? F2", 4, v=>v+0x18);
        sign.AddSignCall("F2 0F 58 ? ? ? ? ? 0F 28 ? E8", 4, v=>v+0x18);
        sign.AddSignCall("F2 0F 10 ? ? ? ? ? E8 ? ? ? ? 0F 57 ? F2 0F 5A ? 4D", 4, v=>v+0x10);
        if(sign.isOK()){
            MinecraftGame.#基址 = sign.get();
            return true;
        }
        return false;
    }
    /**
     * 通过基址直接获取MG指针
     */
    static get Ptr(){
        if(MinecraftGame.#基址 == 0) throw new Error("MinecraftGame 基址为0");
        return new NativePoint(MinecraftGame.#基址).getpoint();
    }
}

MinecraftGame.__init();