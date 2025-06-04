
import SignCode from 'SignCode'
import * as 事件系统 from '事件系统'
import * as 工具 from '工具'

import GuiData from './GuiData';

const logger = new spdlog();

export interface 队列类型回调{
    mainCall:()=>void,
    CheckCall:()=>boolean
}

// 游戏线程执行队列
var 游戏线程执行队列 : 队列类型回调[] = [];

export default class ClientInstance {
    static #Instance : number = 0; 
    static #Vtf : number = 0; // 虚表地址
    static #IsInit : boolean = false;

    static #鼠标位置地址偏移 : number = 0;
    static #Fov所在结构偏移 : number = 0;
    static #Fov地址偏移 : number = 0;
    static #屏幕大小偏移 : number = 0;
    static #到MinecraftGame偏移 : number = 0; // 这个偏移是从ClientInstance到MinecraftGame的偏移
    static #到GuiData的偏移: number = 0;

    static #GetTopScreenCall地址 : number = 0;

    static __init():boolean {
        // 初始化客户端单例
        if (!ClientInstance.#IsInit) {
            ClientInstance.#IsInit = true;
            let sign_tick = new SignCode("ClientInstance_Tick", true);
            sign_tick.AddSign("40 53 48 83 EC ? 48 8B ? 48 8B 89 ? ? ? ? 48 8B ? 48 8B 80 ? ? ? ? FF ? ? ? ? ? 84 ? 75 ? 48");
            if(sign_tick.isOK()){
                // 创建hook
                let ClientInstance_Tick = new HookBase(function (ci_ptr : number){
                    // 广播事件 Tick
                    事件系统.事件广播("onTick", null, ci_ptr);
                    // 跑队列
                    for(let i = 0; i < 游戏线程执行队列.length; i++){
                        if(typeof 游戏线程执行队列[i].CheckCall != 'function' || 游戏线程执行队列[i].CheckCall()){
                            游戏线程执行队列[i].mainCall();
                            游戏线程执行队列.splice(i, 1);
                            i--;
                        }
                    }

                    // 拿到客户端单例
                    if(ClientInstance.#Instance == 0){
                        ClientInstance.#Instance = ci_ptr;
                    }
                    return ClientInstance_Tick.origin.call(ci_ptr);
                }, sign_tick.get(),[NativeTypes.UnsignedLongLong,NativeTypes.UnsignedLongLong]);
                ClientInstance_Tick.hook();
            }
            
            let group1 = ClientInstance.#__findVtf() &&
                // 查找鼠标偏移
                ClientInstance.#__findMousePosOffset() &&
                // 查找Fov偏移
                ClientInstance.#__findFovOffset();

            let group2 = 
                // 查找屏幕大小偏移
                ClientInstance.#__findScreenSizeOffset();

            let group3 = 
                // 查找到MinecraftGame的偏移
                ClientInstance.#__findMinecraftGameOffset();
            
            let grpup4 =
                // 查找到GuiData的偏移
                ClientInstance.#__findGuiDataOffset();

            return group1 && group2 && group3 && grpup4;
        }
        return true;
    }

    /**
     * 添加进
     * @param work 
     */
    static PushNextGameThreadWait(work : 队列类型回调){
        游戏线程执行队列.push(work);
    }

    /**
     * 查找虚表
     * @returns 是否成功
     */
    static #__findVtf():boolean{
        let code = new SignCode("查找CI虚表", true, true);
        code.AddSignCall("48 8D ? ? ? ? ? 48 89 ? 48 8D ? ? ? ? ? 48 89 ? ? 48 8D ? ? ? ? ? 48 89 ? ? ? ? ? 48 8D ? ? ? ? ? 48 89 ? ? ? ? ? 44", 3); // 来自CI构造函数 - 构造函数直接IDA 通过字符串"ScreenLoadTimeTracker TaskGroup"定位
        code.AddSignCall("48 8D ? ? ? ? ? 48 89 ? 48 8D ? ? ? ? ? 48 89 ? ? 48 8D ? ? ? ? ? 48 89 ? ? ? ? ? 48 8D ? ? ? ? ? 48 89 ? ? ? ? ? E8", 3); // 来自CI析构函数
        if(code.isOK()){
            //let _offset = new NativePoint(code.get()).getint();
            //ClientInstance.#Vtf = code.get() + 0x4 + _offset;
            ClientInstance.#Vtf = code.get();
            return true;
        }
        return false;
    }

    /**
     * 查找鼠标位置偏移
     * @returns 是否成功
     */
    static #__findMousePosOffset():boolean{/** vf 0x960 */
        /**
         * 特征码定位方法，首先在CI中找到存储鼠标位置的地址，找写入或访问，都是同一个，然后去到函数，断点找调用，
         * 往上就能看到虚函数偏移，此特征码就是定位此偏移的
         */
        let find = 工具.查找特征码("48 8B 82 ? ? ? ? 0F 5B ? FF");
        if(find == 0) {
            logger.error("未找到鼠标偏移地址,特征码失效");
            return false;
        }
        let 虚表偏移 = (new NativePoint(find + 3)).getint()    // 拿到0x960
        let 写入鼠标位置函数地址 = new NativePoint(ClientInstance.#Vtf + 虚表偏移).getulong();
        ClientInstance.#鼠标位置地址偏移 = new NativePoint(写入鼠标位置函数地址 + 4).getint()
        return true;
    }

    /**
     * 查找fov的偏移
     * @returns 是否成功
     */
    static #__findFovOffset():boolean{
        // old let 鼠标位置到Fov所在结构的偏移= 0x108;     /* 这个0x108是手动计算的，先找到鼠标位置确定地址，然后找到fov，后找访问，提示偏移0x148, 则往上找148，就是这个结构起始点 */
        // old let 未知结构的偏移 = ClientInstance.#鼠标位置地址偏移 + 鼠标位置到Fov所在结构的偏移;
        // 现在通过特征码拿到此结构到fov的偏移，0x148

        // Fov 找访问 第一个 能看到他的逻辑大概就是 使用虚表偏移A执行某个函数，函数内部偏移一定地址，返回一个结构地址（依旧在CI中）
        // 然后将地址加了 148(C)，  
        // 我们通过特征码先拿到这个偏移A， 然后通过A加虚表定位这个函数，拿到其中的偏移B, 然后将B加148(C)，就是Fov的地址
        /*
        Minecraft.Windows.exe+168F7F3 - F3 0F11 8D 7C010000   - movss [rbp+0000017C],xmm1
        Minecraft.Windows.exe+168F7FB - 48 8B 80 A00C0000     - mov rax,[rax+00000CA0(A)]
        Minecraft.Windows.exe+168F802 - FF 15 C0EECD05        - call qword ptr [Minecraft.Windows.exe+736E6C8]
        Minecraft.Windows.exe+168F808 - 48 05 48010000        - add rax,00000148(C)
        Minecraft.Windows.exe+168F80E - F3 0F10 00            - movss xmm0,[rax]
        Minecraft.Windows.exe+168F812 - F3 0F11 85 00010000   - movss [rbp+00000100],xmm0
        Minecraft.Windows.exe+168F81A - F3 0F10 48 04         - movss xmm1,[rax+04]
        Minecraft.Windows.exe+168F81F - F3 0F11 8D 04010000   - movss [rbp+00000104],xmm1
        ......
        特殊函数
        Minecraft.Windows.exe+3FCCC0 - 48 8D 81 F8050000     - lea rax,[rcx+000005F8(B)]
        Minecraft.Windows.exe+3FCCC7 - C3                    - ret 
        */
        let sign = new SignCode("找Fov偏移",true);
        sign.AddSign("F3 0F 11 ? ? ? ? ? 48 8B 80 ? ? ? ? FF 15 ? ? ? ? 48 05");
        //                                        ^(A)                        ^(C)
        if(sign.isOK()){
            let 定位特殊结构偏移函数的虚表偏移_A = new NativePoint(sign.get() + 11).getint();
            let 函数地址 = ClientInstance.#Vtf + 定位特殊结构偏移函数的虚表偏移_A;
            ClientInstance.#Fov所在结构偏移 = new NativePoint(函数地址 + 3).getint();logger.trace()
            let 特殊结构到Fov的偏移_C = new NativePoint(sign.get() + 11 + 12).getint();
            ClientInstance.#Fov地址偏移 = ClientInstance.#Fov所在结构偏移 + 特殊结构到Fov的偏移_C;
            return true;
        }
        return false;
    }
    
    /**
     * 获取屏幕大小的偏移
     * @returns 是否成功
     */
    static #__findScreenSizeOffset():boolean{
        // 屏幕大小的float[] 也在CI中，可以通过调节屏幕大小慢慢找
        // 找到之后找访问 有很多
        // 原理大概是，通过一个虚函数经过一个偏移进行定位这个存放大小的地址
        // 我们通过这个虚表偏移，拿到函数地址，进而拿到CI到屏幕大小的偏移
        /*
            Minecraft.Windows.exe+6647040 - 48 8B 80 E8060000     - mov rax,[rax+000006E8]
            Minecraft.Windows.exe+6647047 - FF 15 7B76D200        - call qword ptr [Minecraft.Windows.exe+736E6C8]
            Minecraft.Windows.exe+664704D - 8B 08                 - mov ecx,[rax]
            Minecraft.Windows.exe+664704F - 8B 40 04              - mov eax,[rax+04]
            Minecraft.Windows.exe+6647052 - 89 43 04              - mov [rbx+04],eax
            ...
            Minecraft.Windows.exe+3ED960 - 48 8D 81 80020000     - lea rax,[rcx+00000280]
            Minecraft.Windows.exe+3ED967 - C3                    - ret 
        */
        let sign = new SignCode("查找屏幕大小偏移", true, true, false);
        sign.AddSign("48 8B 80 ? ? ? ? FF 15 ? ? ? ? 8B ? 8B ? 04 89 ? 04", (v:number)=>{ let a = new NativePoint(v + 3).getint(); return new NativePoint(ClientInstance.#Vtf + a).getpoint().offset(3).toNumber(); });
        sign.AddSign("FF 15 ? ? ? ? F3 41 0F 10 ? ? ? ? ? 48", (v:number)=>v + 11);
        // 下面这个定位的两个都是有效的
        sign.AddSign("48 8B 80 ? ? ? ? FF 15 ? ? ? ? F3 0F ? 00 F3", (v:number)=>{ let a = new NativePoint(v + 3).getint(); return new NativePoint(ClientInstance.#Vtf + a).getpoint().offset(3).toNumber(); });
        sign.AddSign("8B ? 89 81 ? ? ? ? 8B ? 04 89 81 ? ? ? ? 8B ? 08 89 81 ? ? ? ? 8B ? 0C 89 81 ? ? ? ? 8B ? 10 89 81 ? ? ? ? 8B ? 14 89 81 ? ? ? ? C3 ", (v:number)=>v + 4);
        if(sign.isOK()){
            ClientInstance.#屏幕大小偏移 = new NativePoint(sign.get()).getint();
            return true;
        }
        return false;
    }

    /**
     * 拿到Ci 到 MinecraftGame的偏移
     * @returns 是否成功
     */
    static #__findMinecraftGameOffset():boolean{
        // 通过特征码查找
        let sign = new SignCode("定位CI到MG的偏移",true, true, false);
        sign.AddSign("4C 89 ? ? ? ? ? 45 ? ? 4C 89 ? ? ? ? ? 44 88 ? ? ? ? ? 4C", (v:number)=>v+3); // 在CI构造函数中, 第二个参数就是MG
        sign.AddSign("48 8B 8E ? ? ? ? 48 8B 01 48 8B ? ? ? ? ? FF 15 ? ? ? ? 48 8D 0D ? ? ? ? 48 89 8E", (v:number)=>v+3); // 在CI构造函数中, 第二个参数就是MG
        if(sign.isOK()){
            ClientInstance.#到MinecraftGame偏移 = new NativePoint(sign.get()).getint();
            return true;
        }
        return false;
    }

    static #__findGuiDataOffset():boolean{
        // 通过字符串 "ui_invert_overlay" 直接定位到GuiData的构造函数是上，找调用，调用者是CI的虚函数，直接能看到GuiData存到那里了
        let sign = new SignCode("定位CI到GuiData的偏移",true,true,false);
        sign.AddSign("48 8B 8F ? ? ? ? 48 89 87 ? ? ? ? 48 85 ? 74 ? 48 8B ? BA 01 00 00 00 48 8B ? FF 15 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 ? 75 ? 4C 89 3D", (v)=>v+3);
        if(sign.isOK()){
            ClientInstance.#到GuiData的偏移 = new NativePoint(sign.get()).getint();
            return true;
        }
        return false;
    }
    /**
     * 获取客户端单例的指针
     */
    static get Ptr():number{
        // 获取客户端单例
        return ClientInstance.#Instance;
    }

    /**
     * 获取虚表地址
     */
    static get Vtf():number{
        return ClientInstance.#Vtf;
    }
    /**
     * 获取游戏区的鼠标位置
     * @returns 鼠标位置
     */
    static GetMousePos() : vec_2{
        if(ClientInstance.#鼠标位置地址偏移 == 0) throw new Error("存储鼠标位置的偏移为0");
        let mouse = new NativePoint(ClientInstance.#Instance + ClientInstance.#鼠标位置地址偏移);
        return {
            x: mouse.getfloat(),
            y: mouse.offset(4).getfloat()
        };
    }
    /**
     * 获取Fov视野缩放
     * @returns 
     */
    static GetFov() : vec_2{
        if(ClientInstance.#Fov地址偏移 == 0) throw new Error("Fov地址偏移为0");
        let fov = new NativePoint(ClientInstance.#Instance + ClientInstance.#Fov地址偏移);
        return {
            x: fov.getfloat(),
            y: fov.offset(0x14).getfloat()
        };
    }
    /**
     * 获取游戏区屏幕大小
     * @returns 
     */
    static GetScreenSize():vec_2 {
        if(ClientInstance.#屏幕大小偏移 == 0) throw new Error("屏幕大小偏移为0");
        let size = new NativePoint(ClientInstance.#Instance + ClientInstance.#屏幕大小偏移);
        return {
            x: size.getfloat(),
            y: size.offset(4).getfloat()
        };
    }

    /**
     * 获取MinecraftGame实例
     * @returns MinecraftGame实例
     */
    static GetMinecraftGame():NativePoint {
        if(ClientInstance.#到MinecraftGame偏移 == 0) throw new Error("到MinecraftGame的偏移为0");
        return new NativePoint(ClientInstance.#Instance + ClientInstance.#到MinecraftGame偏移).getpoint();
    }

    /**
     * 获取GuiData实例
     * @returns GuiData实例
     */
    static GetGuiData() : GuiData {
        if(ClientInstance.#到GuiData的偏移 == 0) throw new Error("到GuiData的偏移为0");
        return new GuiData(new NativePoint(ClientInstance.#Instance + ClientInstance.#到GuiData的偏移).getpoint());
        //return new NativePoint(ClientInstance.#Instance + ClientInstance.#到GuiData的偏移).getpoint();
    }

    /**
     * 获取当前最顶上UI的名称
     * @returns 
     */
    static GetTopScreenName() : string{
        if(ClientInstance.#GetTopScreenCall地址 == 0){
            let sign = new SignCode("GetTopScreenName", true, true);
            sign.AddSign("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 4C 8B F2 48 8B ? 45 33");
            sign.AddSign("48 8B ? 48 8D 55 ? 48 8B 80 ? ? ? ? FF 15 ? ? ? ? 48 8B ? 48 8B ? 48 8B 81 ? ? ? ? 48 8D 55 ? 48 8B ? FF 15 ? ? ? ? 48 8B ? 48 8B 40",v=>{
                for(let i = 0; i< 500; i++){
                    let byte = new NativePoint(v - i).getuchar();
                    if(byte == 0xCC){
                        return v - i + 1;
                    }
                }
                return 0;
            });
            if(sign.isOK()){
                ClientInstance.#GetTopScreenCall地址 = sign.get();
            }else{
                throw new Error("GetTopScreenName 特征码失效");
            }
        }
        let call = new NativePoint(ClientInstance.#GetTopScreenCall地址);
        call.setAgree([NativeTypes.Pointer, NativeTypes.UnsignedLongLong, NativeTypes.Pointer]);
        let str = new NativePoint(0, 32);   // 申请内存 32
        let ret : NativePoint = call.call(ClientInstance.Ptr, str);
        return ret.getstring();
    }
}

ClientInstance.__init();


// 创建单例hook
//let ClientInstance_Tick = new HookBase()
//export default sign_tick;
