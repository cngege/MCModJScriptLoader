import * as 工具 from '工具'
import * as 事件系统 from '事件系统'
import * as ImGui from 'imgui'
import * as http from 'http'
import GuiData from './module/SDK/GuiData'
import ClientInstanceScreenModel from './module/SDK/ClientInstanceScreenModel'
import ClientInstance from './module/SDK/ClientInstance'
//import {franc} from 'https://esm.sh/franc@6.2.0/es2022/franc.bundle.mjs'
import TrBase, { TrLanguage, LanguateText, isChinese } from './module/Tr/TrBase'
import XunFeiTr from './module/Tr/XunFei/Tr'
import MicrofostTr from './module/Tr/MicrosoftBing/Tr'

(function(模块 : 类型_公共模块){
    let logger = new spdlog();
    // let Trlanguage =  {
    //     "简体中文":1,/**简体中文 */
    //     "英语":2,/**英语 */
    //     "日语":3,/**日语 */
    //     "韩语":4,/**韩语 */
    //     "俄语":5,/**俄语 */
    //     "法语":6, /**法语 */
    //     "西班牙":7, /**西班牙语 */
    //     "阿拉伯":12, /**阿拉伯语 */
    //     "粤语":9, /**粤语 */
    //     "越南语":8, /*越南语 */
    //     "德语":13,/* 德语 */
    //     "意大利":14 /*意大利 */
    // }
    // let TrTextList :string[] =[
    //     "=不翻译=",
    //     "简体中文",//1
    //     "英语",
    //     "日语",
    //     "韩语",
    //     "俄语",
    //     "法语",
    //     "西班牙",
    //     "越南语",
    //     "粤语",//9
    //     "英语",//10
    //     "=不翻译=",
    //     "=不翻译=",
    //     "阿拉伯",
    //     "意大利",
    // ] ;
    var config = {
        开启聊天翻译: true,
        打印耗时: true,
        查询语言时过滤远程玩家发出的消息长度过小: true,
        列出所有玩家检测到的语言: false,/**即用即关 */
        翻译目标语言: LanguateText.英语,
        译后文本带颜色: true,
        发送附带原语言文字: true,
        输出对方使用的语言: true,
        使用回传消息进行测试: false,
        翻译引擎: -1
    };
    var PlayerLanguages = new Map<string, number>();
    // 当前正在查询翻译,还未显示出来的消息数
    var PushCheckTrCountToRemote = 0;
    var PushCheckTrCountToMe = 0;
    var TranslationEngine : TrBase|null = null;

    function createTranslationEngine(id:number){
        if(TranslationEngine && config.翻译引擎 == id) return;
        switch(id){
            case 0: 
                TranslationEngine = new XunFeiTr();
                config.翻译引擎 = id;
                break;
            case 1: 
                TranslationEngine = new MicrofostTr();
                config.翻译引擎 = id;
                break;
            default:
        }
        
    }
    /**
     * 在线查询文本的语言
     * @param str 要查询的文本
     * @param name 哪个玩家的消息
     */
    // function OnlineCheckLanguage(str:string, name:string){
    //     let over : boolean = false;
    //     let ret : http.Response|null = null;
    //     createThread(()=>{
    //         ret = http.Ajax({
    //             url: "https://fanyi.iflyrec.com/TJHZTranslationService/v1/languageDetection",
    //             method:'Post',
    //             headers:{
    //                 'Content-Type': 'application/json',
    //                 'Origin': 'https://fanyi.iflyrec.com',
    //             },
    //             body:{
    //                 originalText:str
    //             },
    //             followlocation: true,
    //         });
    //         over = true;
    //     });
    //     ClientInstance.PushNextGameThreadWait({
    //         mainCall: function(){
    //             if(!ret){
    //                 logger.error("查询玩家语言时 网络请求结果为null")
    //                 return;
    //             }
    //             if(ret.status == 200){
    //                 let v = JSON.parse(ret.body);
    //                 if(v['desc'] == "success"){
    //                     let lgNum = v['biz'][0]['detectionLanguage'];
    //                     PlayerLanguages.set(name, lgNum);
    //                     return;
    //                 }else{
    //                     logger.error("查询玩家语言时 服务器返回消息:", ret.body);
    //                 }
    //             }else{
    //                 logger.error("查询玩家语言时 服务器返回状态码:", ret.status);
    //                 return;
    //             }
    //         },
    //         CheckCall:()=>{
    //             return over;
    //         }
    //     })
    // }

    /**
     * 在线请求翻译
     * @param str 要翻译的字符串
     * @param from 从某语言
     * @param to 翻译到
     * @param call 翻译结果回调
     * @returns 
     */
    // function OnlineTr(str:string, from:number,to:number,call:(str:string,hasErr:boolean,errstr:string)=>void):void{
    //     let over : boolean = false;
    //     let ret : http.Response|null = null;
    //     let ajaxErr:string|null = null;
    //     createThread(()=>{
    //         try{
    //             ret = http.Ajax({
    //                 url: "https://fanyi.iflyrec.com/TJHZTranslationService/v2/textAutoTranslation",
    //                 method:'Post',
    //                 headers:{
    //                     'Content-Type': 'application/json',
    //                     'Origin': 'https://fanyi.iflyrec.com',
    //                 },
    //                 body:{
    //                     from: from,
    //                     to: to,
    //                     type: '1',
    //                     contents:[{
    //                         text: str
    //                     }]
    //                 },
    //                 followlocation: true,
    //                 error: (err)=>{
    //                     ajaxErr = err;
    //                 }
    //             });
    //         }catch(e:any){ajaxErr = e.toString()}
    //         over = true;
    //     });
    //     ClientInstance.PushNextGameThreadWait({
    //         mainCall: function(){
    //             try{
    //                 if(!ret){
    //                     if(ajaxErr != null){
    //                         call(str, true, "翻译错误:"+ajaxErr);
    //                     }else{
    //                         call(str, true, "ret == null");
    //                     }
    //                     return;
    //                 }
    //                 if(ret.status == 200){
    //                     if(ret.body.indexOf("{") != 0){  // 出错
    //                         call(str, true, ret.body);
    //                     }else{
    //                         let v = JSON.parse(ret.body);
    //                         if(v['desc'] == "success"){
    //                             let trStr = v['biz'][0]['sectionResult'][0]['dst'];
    //                             //str.setstring(`${trStr}(原:${sourceMsg})`);
    //                             call(trStr, false, v);
    //                             return;
    //                         }
    //                         call(str, true, v['desc']);
    //                     }
    //                 }else{
    //                     call(str, true, "ret.status"+ret.status);
    //                     return;
    //                 }
    //             }catch(e:any){
    //                 call(str, true, e + ((ret && ret.status == 200)?ret!.body:""));
    //             }
    //         },
    //         CheckCall:()=>{
    //             return over;
    //         }
    //     })
    // }

    /**
     * 文本是不是中文
     * @param text 
     * @param threshold 
     * @param maxCheck 
     * @returns 
     */


    事件系统.监听事件("onSave",function(data : any){
        data[模块.name] = {
            enable: 模块.enable,
            config: config,
            playerLanguagesMap: Object.fromEntries(PlayerLanguages),
        };
        return true;
    });

    事件系统.监听事件("onLoad",function(data : any){
        let thisData = data[模块.name];
        if(thisData){
            模块.enable = thisData.enable;
            if(thisData.config){
                config = thisData.config;
                if(config.翻译引擎){
                    createTranslationEngine(config.翻译引擎);
                }else{
                    config.翻译引擎 = -1;
                }
            }
            if(thisData.playerLanguagesMap){
                try{
                    PlayerLanguages = new Map<string, number>(Object.entries(thisData.playerLanguagesMap));
                }catch(e){
                    logger.error("从配置文件读取玩家语言映射时出错,配置文件可能损坏", e);
                }
            }
        }
    });

    事件系统.监听事件("onRender",()=>{
        if(!模块.enable) return true;
        if(ImGui.Begin(模块.name, (_ = 模块.enable)=>模块.enable = _)){
            ImGui.Text("目前同时查询数量(收)"+PushCheckTrCountToMe,"(发)"+PushCheckTrCountToRemote);
            ImGui.Checkbox("开启聊天翻译",(_ = config.开启聊天翻译)=>config.开启聊天翻译 = _);
            ImGui.Checkbox("打印耗时",(_ = config.打印耗时)=>config.打印耗时 = _);
            ImGui.Checkbox("查询语言过滤短消息",(_ = config.查询语言时过滤远程玩家发出的消息长度过小)=>config.查询语言时过滤远程玩家发出的消息长度过小 = _);
            ImGui.Checkbox("列出所有玩家检测到的语言",(_ = config.列出所有玩家检测到的语言)=>config.列出所有玩家检测到的语言 = _);
            ImGui.Checkbox("翻译后文本带颜色",(_ = config.译后文本带颜色)=>config.译后文本带颜色 = _);
            ImGui.Checkbox("发送附带原语言文字",(_ = config.发送附带原语言文字)=>config.发送附带原语言文字 = _);
            ImGui.Checkbox("输出对方使用的语言",(_ = config.输出对方使用的语言)=>config.输出对方使用的语言 = _);
            ImGui.Checkbox("使用回传消息进行测试",(_ = config.使用回传消息进行测试)=>config.使用回传消息进行测试 = _);

            if(ImGui.RadioButton("讯飞翻译引擎", config.翻译引擎 == 0)){
                createTranslationEngine(0);
            }
            if(ImGui.RadioButton("微软翻译引擎", config.翻译引擎 == 1)){
                createTranslationEngine(1);
            }
            if(TranslationEngine){
                ImGui.Text("当前引擎",TranslationEngine.IsInit()?"已":"未","初始化");
            }else{
                ImGui.Text("还未创建翻译引擎");
            }

            // let click_selectLg = ImGui.Combo("要翻译到的语言",(_ = config.翻译目标语言)=>{
            //     if(_ < 0 ){
            //         config.翻译目标语言 = 1
            //     } 
            //     // else if(TrTextList[_] == "=不翻译="){
            //     //     config.翻译目标语言 = _ + 1
            //     // }
            //     else{
            //         config.翻译目标语言 = _
            //     }
            //     return config.翻译目标语言;
            // }, TrTextList);
            // if(click_selectLg){
            //     logger.debug("翻译目标语言切换到:", TrTextList[config.翻译目标语言]);
            // }

            // if(config.列出所有玩家检测到的语言){
            //     ImGui.Text("点击即可删除,后续等待重新查询");
            //     PlayerLanguages.forEach((value, key) => {
            //         if(value !== undefined && key != ""){
            //             let isclick = ImGui.Selectable(`${key} [${TrTextList[value]}]`,(v)=>false);
            //             if(isclick){
            //                 PlayerLanguages.delete(key);
            //             }
            //         }
            //     });
            // }
            let click_selectLg = ImGui.Combo("要翻译到的语言",(_ = config.翻译目标语言)=>{
                if(_ < 0 ){
                    config.翻译目标语言 = 1
                }
                else{
                    config.翻译目标语言 = _
                }
                return config.翻译目标语言;
            }, TrLanguage.getLanguageTextTabel());
            if(click_selectLg){
                logger.debug("翻译目标语言切换到:", LanguateText[config.翻译目标语言]);
            }

            if(config.列出所有玩家检测到的语言){
                ImGui.Text("点击即可删除,后续等待重新查询");
                PlayerLanguages.forEach((value, key) => {
                    if(value !== undefined && key != ""){
                        let isclick = ImGui.Selectable(`${key} [${LanguateText[value]}]`,(v)=>false);
                        if(isclick){
                            PlayerLanguages.delete(key);
                        }
                    }
                });
            }

            if(ImGui.CollapsingHeader("测试相关")){
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
                if(ImGui.Button("微软翻译测试")){
                    
                }
            }
        }
        ImGui.End();
        return true;
    });

    事件系统.监听事件("onChat",(guidata:number, playername:NativePoint, str:NativePoint, optionalParam:NativePoint, _2:number, _3:number)=>{
        if(模块.enable && config.开启聊天翻译){

            let sourceMsg = str.getstring();
            let sourcePname = playername.getstring();
            if(!TranslationEngine){
                str.setstring(sourceMsg+"(你还未选择一款翻译引擎)")
                return true;
            }
            if(!TranslationEngine.IsInit()){
                str.setstring(sourceMsg+"(该引擎尚未初始化,正在尝试初始化)")
                TranslationEngine.Init();
                return true;
            }
            if(isChinese(sourceMsg)){ // 过滤掉我自己 //TODO:
                return true;
            }
            if(!config.使用回传消息进行测试 && playername.getstring() == "CNGEGE"){
                return true;
            }
            // 判断Map中有没有这个玩家使用的语言
            if(!PlayerLanguages.has(sourcePname) || PlayerLanguages.get(sourcePname) == undefined){
                if(config.查询语言时过滤远程玩家发出的消息长度过小 && sourceMsg.length <= 5){
                    return true;
                }
                if(TranslationEngine.CanAutoTr()){  // 如果支持翻译时候同步查询
                    PlayerLanguages.set(sourcePname, 1); // 1 表示自动检测
                }else if(TranslationEngine.CanFindLanguage()){
                    TranslationEngine.OnlineCheckLanguage(sourceMsg,(sourceText:LanguateText, haserr:boolean, err:string)=>{
                        if(haserr){
                            logger.error(err);
                        }else{
                            PlayerLanguages.set(sourcePname, sourceText);
                        }
                    })
                    str.setstring(sourceMsg+"(开始查询语言..)")
                    PlayerLanguages.set(sourcePname, -1); /** -1 表示正在查询中使用的语言中 */
                    return true;
                }else{
                    str.setstring(sourceMsg+"(无法获悉该用户使用的语言,请切换引擎)")
                    logger.warn("目前使用的翻译引擎既不支持预查询语言类型，也不支持翻译同步查询, 请检查:",config.翻译引擎);
                    return true;
                }
            }
            if(PlayerLanguages.get(sourcePname)! == -1) /** -1 表示正在查询中使用的语言中 */
            {
                str.setstring(sourceMsg+"(正在查询语言..)")
                return true;
            }

            logger.info(`[聊天] [${sourcePname}]`,sourceMsg);
            let 开始时间: number = Date.now();
            PushCheckTrCountToMe ++;
            TranslationEngine.OnlineTr(sourceMsg,/*Trlanguage.英语 *这里要进行自动判断语言 */PlayerLanguages.get(sourcePname)!,LanguateText.简体中文, (trStr:string,lg:LanguateText, hasErr:boolean, infoStr:string)=>{
                PushCheckTrCountToMe--;
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
                if(PlayerLanguages.has(sourcePname)){
                    if(lg != LanguateText.不支持){
                        PlayerLanguages.set(sourcePname, lg);
                    }
                }
                let ms = "";
                if(config.打印耗时){
                    let 毫秒数 = Date.now() - 开始时间;
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

                let 文本 = `${trStr}(原${ms}:${sourceMsg})`;
                if(config.译后文本带颜色){
                    文本 = `§6${trStr}§f(原${ms}:§a${sourceMsg}§f)`;
                }
                if(config.输出对方使用的语言){
                    if(PlayerLanguages.has(sourcePname)){
                        let index = PlayerLanguages.get(sourcePname);
                        if(index !== undefined){
                            文本 += `[${LanguateText[index]}]`
                        }
                    }
                }
                newstr.setstring(文本);
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
        //logger.debug("CIM:",CIScreenModel.toString(16));
        if(模块.enable && config.开启聊天翻译){
            if(!TranslationEngine){
                logger.warn("请选择一个翻译引擎");
                return true;
            }
            if(!TranslationEngine?.IsInit()){
                logger.warn("该翻译引擎未初始化，正在尝试初始化, 请几秒后再试");
                TranslationEngine?.Init();
                return true;
            }
            let sourceMsg = str.getstring();
            let 开始时间: number = Date.now();
            if(!isChinese(sourceMsg)){
                return true;
            }
            if(config.翻译目标语言 == LanguateText.不支持) return true;
            PushCheckTrCountToRemote ++;
            TranslationEngine.OnlineTr(sourceMsg,LanguateText.简体中文,config.翻译目标语言, (trStr:string, lg: LanguateText, hasErr:boolean, infoStr:string)=>{
                PushCheckTrCountToRemote--;
                if(hasErr){
                    ClientInstance.GetGuiData().displayClientMessage("翻译聊天消息出错§7"+infoStr);
                    return;
                }
                let ms = "";
                if(config.打印耗时){
                    let 毫秒数 = Date.now() - 开始时间;
                    if(毫秒数 > 1000){
                        毫秒数 = (毫秒数 / 1000.0);
                        ms = "<"+毫秒数.toString() + "s>";
                    }else{
                        ms = "<"+毫秒数.toString() + "ms>";
                    }
                }
                let 原始文本 = "";
                if(config.发送附带原语言文字){
                    原始文本 = `(原${ms}:${sourceMsg})`;
                }
                if(config.译后文本带颜色){
                    trStr = `§6${trStr}§r`;
                    if(config.发送附带原语言文字){
                        原始文本 = `(原${ms}:§a${sourceMsg}§r)`;
                    }
                }
                let 全文本 = `${trStr}${原始文本}`;
                let newstr = new NativePoint(工具.申请CPP字符串());
                newstr.setstring(全文本);
                new ClientInstanceScreenModel(CIScreenModel).sendChatMessage(newstr);
                工具.释放CPP字符串(newstr.toNumber());
            });
            return false;//拦截
        }
        return true;
    })


})(__模块__)