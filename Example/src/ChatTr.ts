import * as 工具 from '工具'
import * as 事件系统 from '事件系统'
import * as ImGui from 'imgui'
import * as http from 'http'
import GuiData from './module/SDK/GuiData'
import ClientInstanceScreenModel from './module/SDK/ClientInstanceScreenModel'
import ClientInstance from './module/SDK/ClientInstance'
//import {franc} from 'https://esm.sh/franc@6.2.0/es2022/franc.bundle.mjs'

(function(模块 : 类型_公共模块){
    let logger = new spdlog();
    let Trlanguage =  {
        "简体中文":1,/**简体中文 */
        "英语":2,/**英语 */
        "日语":3,/**日语 */
        "韩语":4,/**韩语 */
        "俄语":5,/**俄语 */
        "法语":6, /**法语 */
        "西班牙":7, /**西班牙语 */
        "阿拉伯":12, /**阿拉伯语 */
        "粤语":9, /**粤语 */
        "越南语":8, /*越南语 */
        "德语":13,/* 德语 */
        "意大利":14 /*意大利 */
    }
    var config = {
        开启聊天翻译: true,
        打印耗时: true,
        查询语言时过滤远程玩家发出的消息长度过小: true,
        列出所有玩家检测到的语言: false,/**即用即关 */
        翻译目标语言: Trlanguage.英语,
    };
    var PlayerLanguages = new Map<string, number>();

    /**
     * 在线查询文本的语言
     * @param str 要查询的文本
     * @param name 哪个玩家的消息
     */
    function OnlineCheckLanguage(str:string, name:string){
        let over : boolean = false;
        let ret : http.Response|null = null;
        createThread(()=>{
            ret = http.Ajax({
                url: "https://fanyi.iflyrec.com/TJHZTranslationService/v1/languageDetection",
                method:'Post',
                headers:{
                    'Content-Type': 'application/json',
                    'Origin': 'https://fanyi.iflyrec.com',
                },
                body:{
                    originalText:str
                },
                followlocation: true,
            });
            over = true;
        });
        ClientInstance.PushNextGameThreadWait({
            mainCall: function(){
                if(!ret){
                    logger.error("查询玩家语言时 网络请求结果为null")
                    return;
                }
                if(ret.status == 200){
                    let v = JSON.parse(ret.body);
                    if(v['desc'] == "success"){
                        let lgNum = v['biz'][0]['detectionLanguage'];
                        PlayerLanguages.set(name, lgNum);
                        return;
                    }else{
                        logger.error("查询玩家语言时 服务器返回消息:", ret.body);
                    }
                }else{
                    logger.error("查询玩家语言时 服务器返回状态码:", ret.status);
                    return;
                }
            },
            CheckCall:()=>{
                return over;
            }
        })
    }

    /**
     * 在线请求翻译
     * @param str 要翻译的字符串
     * @param from 从某语言
     * @param to 翻译到
     * @param call 翻译结果回调
     * @returns 
     */
    function OnlineTr(str:string, from:number,to:number,call:(str:string,hasErr:boolean,debuginfo:string)=>void):void{
        let over : boolean = false;
        let ret : http.Response|null = null;
        createThread(()=>{
            ret = http.Ajax({
                url: "https://fanyi.iflyrec.com/TJHZTranslationService/v2/textAutoTranslation",
                method:'Post',
                headers:{
                    'Content-Type': 'application/json',
                    'Origin': 'https://fanyi.iflyrec.com',
                },
                body:{
                    from: from,
                    to: to,
                    type: '1',
                    contents:[{
                        text: str
                    }]
                },
                followlocation: true,
            });
            over = true;
        });
        ClientInstance.PushNextGameThreadWait({
            mainCall: function(){
                if(!ret){
                    call(str, true, "ret == null");
                    return;
                }
                if(ret.status == 200){
                    let v = JSON.parse(ret.body);
                    if(v['desc'] == "success"){
                        let trStr = v['biz'][0]['sectionResult'][0]['dst'];
                        //str.setstring(`${trStr}(原:${sourceMsg})`);
                        call(trStr, false, v);
                        return;
                    }
                    call(str, true, v['desc']);
                }else{
                    call(str, true, "ret.status"+ret.status);
                    return;
                }
            },
            CheckCall:()=>{
                return over;
            }
        })
    }
    /**
     * 文本是不是中文
     * @param text 
     * @param threshold 
     * @param maxCheck 
     * @returns 
     */
    function isChinese(text:string, threshold = 0.5, maxCheck = 100):Boolean {
        if (!text) return false;
        const sampleText = text.length > maxCheck ? text.substring(0, maxCheck) : text;
        let totalCount = sampleText.length;
        let chineseCount = 0;
        for (let char of sampleText) {
            if (char.match(/[\u3400-\u4DBF\u4E00-\u9FFF]/)) {
                chineseCount++;
            }
        }
        return (chineseCount / totalCount) > threshold;
    }

    事件系统.监听事件("onSave",function(data : any){
        data[模块.name] = {
            enable: 模块.enable,
            config: config,
        };
        return true;
    });

    
    let 翻译目标语言索引 = -1;
    事件系统.监听事件("onLoad",function(data : any){
        let thisData = data[模块.name];
        if(thisData){
            模块.enable = thisData.enable;
            config = config;
        }
        for(let i = 0; i < Object.values(Trlanguage).length;i++){
            if(Object.values(Trlanguage)[i] == config.翻译目标语言){
                翻译目标语言索引 = i;
            }
        }
        
    });

    事件系统.监听事件("onRender",()=>{
        if(!模块.enable) return true;
        if(ImGui.Begin(模块.name, (_ = 模块.enable)=>模块.enable = _)){
            ImGui.Checkbox("开启聊天翻译",(_ = config.开启聊天翻译)=>config.开启聊天翻译 = _);
            ImGui.Checkbox("打印耗时",(_ = config.打印耗时)=>config.打印耗时 = _);
            ImGui.Checkbox("查询语言过滤短消息",(_ = config.查询语言时过滤远程玩家发出的消息长度过小)=>config.查询语言时过滤远程玩家发出的消息长度过小 = _);
            ImGui.Checkbox("列出所有玩家检测到的语言",(_ = config.列出所有玩家检测到的语言)=>config.列出所有玩家检测到的语言 = _);

            if(ImGui.Combo("要翻译到的语言",(_ = 翻译目标语言索引)=>翻译目标语言索引 = _, Object.keys(Trlanguage))){
                config.翻译目标语言 = Object.values(Trlanguage)[翻译目标语言索引];
                logger.debug("翻译目标语言切换到:", config.翻译目标语言);
            }

            ImGui.Text("测试文本");
            if(ImGui.Button("测试文本是什么语言")){
                //logger.info("语言：", franc("测试文本测试文本测试文本测试文本"));
                ClientInstanceScreenModel.name.toUpperCase();
            }
            if(ImGui.Button("查询语言")){
                createThread(()=>{
                    let _lg = ["这是一段中文", "this is a english", "ありがとうございます","만나서 반갑습니다"];
                    for(let i=0;i<_lg.length;i++){
                        let ret = http.Ajax({
                            url: "https://fanyi.iflyrec.com/TJHZTranslationService/v1/languageDetection",
                            method:'Post',
                            headers:{
                                'Content-Type': 'application/json',
                                'Origin': 'https://fanyi.iflyrec.com',
                            },
                            body:{
                                originalText:_lg[i]
                            },
                            followlocation: true,
                        });
                        if(ret && ret.status == 200){
                            let v = JSON.parse(ret.body);
                            if(v['desc'] == "success"){
                                logger.info(`第${i}段文本语言号是`, v['biz'][0]['detectionLanguage']);
                            }else{
                                logger.error("请求失败",ret.body);
                            }
                        }else{
                            logger.error("请求失败");
                        }
                    }
                });
            }
        }
        ImGui.End();
        return true;
    });

    事件系统.监听事件("onChat",(guidata:number, playername:NativePoint, str:NativePoint, optionalParam:NativePoint, _2:number, _3:number)=>{
        if(config.开启聊天翻译){
            // 将原始的聊天信息记录到日志中: 就是不能记录发送者
            let sourceMsg = str.getstring();
            let sourcePname = playername.getstring();
            if(playername.getstring() == "CNGEGE" || isChinese(sourceMsg)){ // 过滤掉我自己 //TODO:
                return true;
            }
            // 判断Map中有没有这个玩家使用的语言
            if(!PlayerLanguages.has(sourcePname) || PlayerLanguages.get(sourcePname) == undefined){
                if(config.查询语言时过滤远程玩家发出的消息长度过小 && sourceMsg.length <= 5){
                    return true;
                }
                OnlineCheckLanguage(sourceMsg, sourcePname); // 检测语言并写道Map中
                str.setstring(sourceMsg+"(开始查询语言..)")
                PlayerLanguages.set(sourcePname, -1);
                return true;
            }
            if(PlayerLanguages.get(sourcePname)! <= 0) /** -1 表示正在查询中使用的语言中 */
            {
                str.setstring(sourceMsg+"(正在查询语言..)")
                return true;
            }

            logger.info(`[聊天] [${sourcePname}]`,sourceMsg);
            let 开始时间: number = new Date().getMilliseconds();
            OnlineTr(sourceMsg,/*Trlanguage.英语 *这里要进行自动判断语言 */PlayerLanguages.get(sourcePname)!,Trlanguage.简体中文, (trStr:string, hasErr:boolean, infoStr:string)=>{
                if(hasErr){
                    //ClientInstance.GetGuiData().displayClientMessage("翻译聊天消息出错§7"+infoStr);
                    let plname = new NativePoint(工具.申请CPP字符串());
                    plname.setstring(sourcePname);
                    let newstr = new NativePoint(工具.申请CPP字符串());
                    newstr.setstring(`翻译出错(原:${sourceMsg})`);
                    let _optionalParam = new NativePoint(0, 40);
                    _optionalParam.offset(32).setbool(false);
                    ClientInstance.GetGuiData().displayChatMessage(plname, newstr, _optionalParam,_optionalParam.toNumber(),_optionalParam.toNumber());
                    工具.释放CPP字符串(newstr.toNumber());
                    工具.释放CPP字符串(plname.toNumber());
                    logger.error("翻译出错:",infoStr);
                    return;
                }
                let ms = "";
                if(config.打印耗时){
                    let 毫秒数 = (new Date().getMilliseconds() - 开始时间);
                    if(毫秒数 > 1000){
                        毫秒数 = (毫秒数 / 1000.0);
                        ms = "<"+毫秒数.toString() + "s>";
                    }else{
                        ms = "<"+毫秒数.toString() + "ms>";
                    }
                }
                
                let plname = new NativePoint(工具.申请CPP字符串());
                plname.setstring(sourcePname);
                let newstr = new NativePoint(工具.申请CPP字符串());
                newstr.setstring(`${trStr}(原${ms}:${sourceMsg})`);
                let _optionalParam = new NativePoint(0, 40);
                _optionalParam.offset(32).setbool(false);
                ClientInstance.GetGuiData().displayChatMessage(plname, newstr, _optionalParam, _optionalParam.toNumber(), _optionalParam.toNumber());
                工具.释放CPP字符串(newstr.toNumber());
                工具.释放CPP字符串(plname.toNumber());
            });
            return false;
        }
        return true;
    })



    事件系统.监听事件("OnSendMessage", (CIScreenModel: number, str:NativePoint)=>{
        // 测试 一律翻译为英语同步发送
        let sourceMsg = str.getstring();
        let 开始时间: number = new Date().getMilliseconds();
        if(config.开启聊天翻译 && isChinese(sourceMsg)){
            OnlineTr(sourceMsg,Trlanguage.简体中文,config.翻译目标语言, (trStr:string, hasErr:boolean, infoStr:string)=>{
                if(hasErr){
                    ClientInstance.GetGuiData().displayClientMessage("翻译聊天消息出错§7"+infoStr);
                    return;
                }
                let ms = "";
                if(config.打印耗时){
                    let 毫秒数 = (new Date().getMilliseconds() - 开始时间);
                    if(毫秒数 > 1000){
                        毫秒数 = (毫秒数 / 1000.0);
                        ms = "<"+毫秒数.toString() + "s>";
                    }else{
                        ms = "<"+毫秒数.toString() + "ms>";
                    }
                }
                let newstr = new NativePoint(工具.申请CPP字符串());
                newstr.setstring(`${trStr}(原${ms}:${sourceMsg})`);
                new ClientInstanceScreenModel(CIScreenModel).sendChatMessage(newstr);
                工具.释放CPP字符串(newstr.toNumber());
            });
            return false;//拦截
        }
        return true;
    })


})(__模块__)