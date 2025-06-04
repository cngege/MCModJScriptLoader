import SignCode from 'SignCode'
import * as 工具 from '工具'
import * as 事件系统 from '事件系统'
//import ClientInstance from './ClientInstance';

const logger = new spdlog();

export default class GuiData extends NativePoint{
    static #IS_Init = false;
    static #Hook_显示聊天消息 : HookBase|null = null;

    static #函数地址_显示客户端消息 : number = 0;

    constructor(ptr : number | NativePoint){
        if(typeof ptr === "number"){
            super(ptr);
        }else{
            super(ptr.toNumber());
        }
    }
    
    static __init(){
        if(!GuiData.#IS_Init){
            GuiData.#IS_Init = true;

            // Hook接受消息函数 GuiData::displayChatMessage
            let sign = new SignCode("接受聊天消息hook",true,true,true);
            sign.AddSign("40 55 53 56 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B F1 49 8B F8");
            sign.AddSignCall("E8 ? ? ? ? 90 48 85 FF 74 ? BB ? ? ? ? 8B C3 F0 0F C1 47 ? 83 F8 ? 75 ? 48 8B 07 48 8B CF 48 8B 00 FF 15 ? ? ? ? F0 0F C1 5F ? 83 FB ? 75 ? 48 8B 07 48 8B CF 48 8B 40 ? FF 15 ? ? ? ? 90 48 8D 4D ? E9 ? ? ? ? 49 8B 4F ? 48 8B 01 48 8D 55 ? 48 8B 80 ? ? ? ? FF 15 ? ? ? ? 48 8B C8 48 8B 00 48 85 C0 0F 84 ? ? ? ? 80 38 ? 0F 84 ? ? ? ? 48 8B 41 ? 48 85 C0 74 ? F0 FF 40 ? 48 8B 01 48 89 45 ? 48 8B 79 ? 48 89 7D ? 48 8B 49 ? 48 89 4D ? 4C 8D 85")
            if(sign.isOK()){
                GuiData.#Hook_显示聊天消息 = new HookBase((guidata:number, playername:NativePoint,str:NativePoint, optionalParam:NativePoint,a5:number,a6:number)=>{
                    let 执行原函数 = true;
                    事件系统.事件广播("onChat",(canCall:boolean)=>{执行原函数 = canCall},guidata,playername,str,optionalParam,a5,a6);
                    if(执行原函数){
                        //let sourceMsg = str.getstring();
                        //str.setstring("此聊天消息被我人为干预["+sourceMsg+"]");
                        return GuiData.#Hook_显示聊天消息!.origin.call(guidata,playername,str,optionalParam,a5,a6);
                    }
                    return 0;
                },sign.get(), [NativeTypes.UnsignedLongLong,
                    NativeTypes.UnsignedLongLong,//GuiData
                    NativeTypes.Pointer,// 玩家名字符串指针
                    NativeTypes.Pointer,// 字符串指针
                    NativeTypes.Pointer,//optionalParam
                    NativeTypes.UnsignedLongLong,//可能是指向0的指针
                    NativeTypes.UnsignedLongLong// ken
                ])
                GuiData.#Hook_显示聊天消息.hook();
            }
        }
    }

    /**
     * 显示客户端聊天消息（从Hook的origin）
     * @param playername 玩家名
     * @param str 字符串指针
     * @param optionalParam 字符串额外参数指针
     * @param unknow2 未知
     * @param unknow3 未知
     * @returns 未知指针
     */
    displayChatMessage(playername:NativePoint, str: NativePoint, optionalParam: NativePoint, unknow2: number, unknow3: number):number{
        if(GuiData.#Hook_显示聊天消息){
            return GuiData.#Hook_显示聊天消息.origin.call(this.toNumber(),playername,str,optionalParam,unknow2,unknow3);
        }
        return 0;
    }

    /**
     * 显示客户端消息
     * @param message 消息内容
     * @param optionalParam 可选参数, 可以是字符串或null
     * @param someFlag 可选标志, 默认为null
     */
    public displayClientMessage(message : string, optionalParam? : string | null, someFlag? : boolean | null) : void{
        // logger.info("cs:", message);
        // return;
        if(GuiData.#函数地址_显示客户端消息 == 0){
            // 搜索字符串 persona.emote.chat_message 找到字符串所在的函数,重下往上找, 大概找到 四参数，最后一个参数为1 第三个参数是引用的就是
            // void GuiData::displayClientMessage(const std::string& message, const std::optional<std::string> optionalParam, bool someFlag)
            let sign = new SignCode("输出客户端文本消息", true, true, true);
            sign.AddSign("40 55 53 56 57 41 56 48 8D AC ? ? ? ? ? 48 81 ? ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 41 0F B6 F9 49 8B");
            //sign.AddSignCall("41 B1 ? 4C 8D 45 ? 48 8D 55 ? E8 ? ? ? ? 90 80 7D ? 00 74 ? 48 8B 55 ? 48 83 FA ? 76 ? 48 FF C2 48 8B 4D ? 48 8B C1 48 81 FA 00 10 00 00", 12);
            sign.AddSignCall("4C 8D 45 ? 48 8D 55 ? E8 ? ? ? ? 90 80 7D ? 00 74", 9);
            sign.AddSignCall("48 8D 95 ? ? ? ? E8 ? ? ? ? 90 48 85 FF 74 ? BB", 8);
            sign.AddSignCall("4D 8B C6 48 8B D7 E8 ? ? ? ? 90 BE FF FF FF FF", 7);
            if(sign.isOK()){
                GuiData.#函数地址_显示客户端消息 = sign.get();
            }else{
                throw new Error("无法获取GuiData::displayClientMessage函数地址, 特征码失效");
            }
        }
        
        /*
        +--------------------------------------------------+
        | 标志（0） |       7字节填充       | 32字节未初始化数据 |
        +--------------------------------------------------+
        0x7ffe10     1B                7B              32B
        */
       /**
        内存地址增加方向
        ▼
        ┌────────────────────────────────┬────┬────────────────┐
        │        T (std::string 值)       │ bool│   对齐填充      │
        │           32 字节               │ 1字 │    7 字节      │
        └────────────────────────────────┴────┴────────────────┘
                    ↑                   ↑
                    值区域 (在前)         状态标志 (在后)
        */
        // let _optionalParam = new NativePoint(0, 1+7+32);
        // if(optionalParam){
        //     _optionalParam.setbool(true);
        //     _optionalParam.offset(8).setstring(optionalParam!);
        // }else{
        //     _optionalParam.setbool(false);
        // }
        let _optionalParam = new NativePoint(0, 1+7+32);
        // _optionalParam.onfree((v)=>{
            
        // });
        if(optionalParam){
            _optionalParam.offset(32).setbool(true);
            _optionalParam.setstring(optionalParam);
        }else{
            _optionalParam.offset(32).setbool(false);
        }
        let call = new NativePoint(GuiData.#函数地址_显示客户端消息);
        call.setAgree([NativeTypes.Void, NativeTypes.UnsignedLongLong, NativeTypes.Pointer, NativeTypes.Pointer, NativeTypes.Bool]);
        let _message = new NativePoint(0, 32);
        _message.setstring(message);
        call.call(this.toNumber(), _message, _optionalParam, someFlag === true);
    }
}

GuiData.__init();