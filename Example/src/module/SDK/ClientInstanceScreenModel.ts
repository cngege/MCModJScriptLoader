import SignCode from 'SignCode'
import * as 工具 from '工具'
import * as 事件系统 from '事件系统'
//import ClientInstance from './ClientInstance';

const logger = new spdlog();

export default class ClientInstanceScreenModel extends NativePoint{
    static #IS_Init = false;
    static #Hook_发送聊天消息 : HookBase|null = null;

    constructor(ptr : number | NativePoint){
        if(typeof ptr === "number"){
            super(ptr);
        }else{
            super(ptr.toNumber());
        }
    }
    
    static __init(){
        if(!ClientInstanceScreenModel.#IS_Init){
            ClientInstanceScreenModel.#IS_Init = true;
            let sign = new SignCode("本地发送聊天消息地址", true, true, true);
            sign.AddSign("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 8B EA 48 8B F9");
            sign.AddSignCall("E8 ? ? ? ? 3C ? 75 ? 48 8B 8F");
            if(sign.isOK()){
                //ClientInstanceScreenModel::sendChatMessage
                ClientInstanceScreenModel.#Hook_发送聊天消息 = new HookBase((a1:number, str:NativePoint)=>{
                    let 拦截 = false;
                    事件系统.事件广播("OnSendMessage",(ret:boolean)=>{
                        if(ret == false) 拦截 = true;
                    },a1,str);
                    if(!拦截){
                        return ClientInstanceScreenModel.#Hook_发送聊天消息!.origin.call(a1, str);
                    }
                    return false;
                },sign.get(), [ NativeTypes.Bool,
                    NativeTypes.UnsignedLongLong,
                    NativeTypes.Pointer
                ])
                ClientInstanceScreenModel.#Hook_发送聊天消息.hook();
            }else{
                throw new Error("ClientInstanceScreenModel 初始化失败, 本地发送聊天消息地址特征码失效");
            }
        }
    }


    /**
     * 发送客户端消息
     * @param message 消息内容
     */
    public sendChatMessage(message : NativePoint) : boolean{
        if(ClientInstanceScreenModel.#Hook_发送聊天消息)
            return ClientInstanceScreenModel.#Hook_发送聊天消息?.origin.call(this.toNumber(), message);
        else
            return false;
    }
}

ClientInstanceScreenModel.__init();