import SignCode from 'SignCode'
import * as 工具 from '工具'
//import ClientInstance from './ClientInstance';

const logger = new spdlog();

export default class GuiData extends NativePoint{
    static #显示客户端消息函数地址 : number = 0;

    constructor(ptr : number | NativePoint){
        if(typeof ptr === "number"){
            super(ptr);
        }else{
            super(ptr.toNumber());
        }
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
        if(GuiData.#显示客户端消息函数地址 == 0){
            // 搜索字符串 persona.emote.chat_message 找到字符串所在的函数,重下往上找, 大概找到 四参数，最后一个参数为1 第三个参数是引用的就是
            // void GuiData::displayClientMessage(const std::string& message, const std::optional<std::string> optionalParam, bool someFlag)
            let sign = new SignCode("输出客户端文本消息", true, true, true);
            sign.AddSign("40 55 53 56 57 41 56 48 8D AC ? ? ? ? ? 48 81 ? ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 41 0F B6 F9 49 8B");
            //sign.AddSignCall("41 B1 ? 4C 8D 45 ? 48 8D 55 ? E8 ? ? ? ? 90 80 7D ? 00 74 ? 48 8B 55 ? 48 83 FA ? 76 ? 48 FF C2 48 8B 4D ? 48 8B C1 48 81 FA 00 10 00 00", 12);
            sign.AddSignCall("4C 8D 45 ? 48 8D 55 ? E8 ? ? ? ? 90 80 7D ? 00 74", 9);
            sign.AddSignCall("48 8D 95 ? ? ? ? E8 ? ? ? ? 90 48 85 FF 74 ? BB", 8);
            sign.AddSignCall("4D 8B C6 48 8B D7 E8 ? ? ? ? 90 BE FF FF FF FF", 7);
            if(sign.isOK()){
                GuiData.#显示客户端消息函数地址 = sign.get();
            }else{
                throw new Error("无法获取GuiData::displayClientMessage函数地址, 特征码失效");
            }
        }
        
        let _message = new NativePoint(0, 32);
        _message.setstring(message);
        /*
        +--------------------------------------------------+
        | 标志（0） |       7字节填充       | 32字节未初始化数据 |
        +--------------------------------------------------+
        0x7ffe10     1B                7B              32B
        */
        let _optionalParam = new NativePoint(0, 1+7+32);
        if(optionalParam){
            _optionalParam.setbool(true);
            _optionalParam.offset(8).setstring(optionalParam!);
        }else{
            _optionalParam.setbool(false);
        }
        let call = new NativePoint(GuiData.#显示客户端消息函数地址);
        call.setAgree([NativeTypes.Void, NativeTypes.UnsignedLongLong, NativeTypes.Pointer, NativeTypes.Pointer, NativeTypes.Bool]);
        call.call(this.toNumber(), _message, _optionalParam, someFlag === true);
    }
}