import { TrLanguage,LanguateText } from '../TrBase'
import TrBase from '../TrBase'
import * as http from 'http'
import ClientInstance from '../../SDK/ClientInstance';

let logger = new spdlog();
export default class XunFeiTr extends TrBase{
    private static languageTabel: Record<LanguateText,number> = {
        [LanguateText.不支持] : -2,
        [LanguateText.自动检查] : -1,
        [LanguateText.简体中文]: 1,/**简体中文 */
        [LanguateText.英语]: 2,/**英语 */
        [LanguateText.日语]: 3,/**日语 */
        [LanguateText.韩语]: 4,/**韩语 */
        [LanguateText.俄语]: 5,/**俄语 */
        [LanguateText.法语]: 6,/**法语 */
        [LanguateText.西班牙]: 7, /**西班牙语 */
        [LanguateText.阿拉伯]: 12,/**阿拉伯语 */
        [LanguateText.粤语]: 9, /**粤语 */
        [LanguateText.越南语]: 8, /*越南语 */
        [LanguateText.德语]: 13, /**德语 */
        [LanguateText.意大利]: 14, /*意大利 */
    }
    /**
     * 是否初始化了（可能初始化失败） XUNFEI 不需要初始化
     */
    protected isInit : boolean = true;

    constructor(){
        super();
    }

    /**
     * 支持专门去查询文字的语言
     * @returns 
     */
    public CanFindLanguage():boolean{
        return true;
    }
    /**
     * 不支持自动识别语言进行翻译到目标语言
     * @returns 
     */
    public CanAutoTr():boolean{
        return false;
    }

    public getInternalLanguageCode(id: LanguateText):number|null{
        return XunFeiTr.languageTabel[id]??null;
    }
    /**
     * （不要高频调用）根据 此引擎中的语言标识“en”转为number
     * @param lg 
     * @returns 如果为null 表示不存在
     */
    public static getCodeByInternalLanguage(lg: number):LanguateText|null{
        let ret:LanguateText | null = null;
        Object.entries(XunFeiTr.languageTabel).forEach(([key, value]) => {
            if(value == lg){
                ret = Number(key);
                return;
            }
        });
        return ret;
    }

    /**
     * 在线查询文本的语言
     * @param str 要查询的文本
     * @param call 查询结果回调
     */
    public OnlineCheckLanguage(str:string, call:(lg:LanguateText, haserr:boolean, err:string)=>void){
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
                    //logger.error()
                    call(0,true, "查询玩家语言时 网络请求结果为null");
                    return;
                }
                if(ret.status == 200){
                    let v = JSON.parse(ret.body);
                    if(v['desc'] == "success"){
                        let lgNum = v['biz'][0]['detectionLanguage'];
                        //PlayerLanguages.set(name, lgNum);
                        let lgcode = XunFeiTr.getCodeByInternalLanguage(lgNum)??0;
                        if(lgcode == 0){
                            call(0, true, "XunFeiTr.getCodeByInternalLanguage 查询结果失败, 网络查询值:"+lgNum);
                        }else{
                            call(lgcode, false, "");
                        }
                        return;
                    }else{
                        call(0,true, "查询玩家语言时 服务器返回消息:"+ret.body);
                        return;
                    }
                }else{
                    call(0,true, "查询玩家语言时 服务器返回状态码:"+ret.status);
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
    public OnlineTr(str:string, from:number,to:number,call:(str:string,sourceLg:LanguateText,hasErr:boolean,errstr:string)=>void):void{
        let over : boolean = false;
        let ret : http.Response|null = null;
        let ajaxErr:string|null = null;

        let fromlg: number|null = this.getInternalLanguageCode(from);
        let tolg :number|null = this.getInternalLanguageCode(to);
        if(fromlg == null){
            call("",0,true,"要翻译的文本语言不支持"+from.toString())
            return;
        }
        if(tolg == null){
            call("",0,true,"目标语言不支持"+to.toString())
            return;
        }

        createThread(()=>{
            try{
                ret = http.Ajax({
                    url: "https://fanyi.iflyrec.com/TJHZTranslationService/v2/textAutoTranslation",
                    method:'Post',
                    headers:{
                        'Content-Type': 'application/json',
                        'Origin': 'https://fanyi.iflyrec.com',
                    },
                    body:{
                        from: fromlg,
                        to: tolg,
                        type: '1',
                        contents:[{
                            text: str
                        }]
                    },
                    followlocation: true,
                    error: (err)=>{
                        ajaxErr = err;
                    }
                });
            }catch(e:any){ajaxErr = e.toString()}
            over = true;
        });
        ClientInstance.PushNextGameThreadWait({
            mainCall: function(){
                try{
                    if(!ret){
                        if(ajaxErr != null){
                            call(str, 0, true, "翻译错误:"+ajaxErr);
                        }else{
                            call(str, 0, true, "ret == null");
                        }
                        return;
                    }
                    if(ret.status == 200){
                        if(ret.body.indexOf("{") != 0){  // 出错
                            call(str, 0, true, ret.body);
                        }else{
                            let v = JSON.parse(ret.body);
                            if(v['desc'] == "success"){
                                let trStr = v['biz'][0]['sectionResult'][0]['dst'];
                                //str.setstring(`${trStr}(原:${sourceMsg})`);
                                call(trStr, 0, false, v);
                                return;
                            }
                            call(str, 0, true, v['desc']);
                        }
                    }else{
                        call(str, 0, true, "ret.status"+ret.status);
                        return;
                    }
                }catch(e:any){
                    call(str, 0, true, e + ((ret && ret.status == 200)?ret!.body:""));
                }
            },
            CheckCall:()=>{
                return over;
            }
        })
    }
}