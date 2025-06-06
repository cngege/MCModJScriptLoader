import { TrLanguage, LanguateText } from '../TrBase'
import TrBase from '../TrBase'
import * as http from 'http'
import ClientInstance from '../../SDK/ClientInstance';

let logger = new spdlog();
export default class MicrofostTr extends TrBase{
    private token:string = "";
    private key:number = 0;
    /**
     * token有效时间
     */
    private tokenTime = 0;

    private static languageTabel: Record<LanguateText,string> = {
        [LanguateText.自动检查] : "auto-detect",
        [LanguateText.不支持] : "",
        [LanguateText.简体中文]: "zh-Hans",/**简体中文 */
        [LanguateText.英语]: "en",/**英语 */
        [LanguateText.日语]: "ja",/**日语 */
        [LanguateText.韩语]: "ko",/**韩语 */
        [LanguateText.俄语]: "ru",/**俄语 */
        [LanguateText.法语]: "fr",/**法语 */
        [LanguateText.西班牙]: "es", /**西班牙语 */
        [LanguateText.阿拉伯]: "ar",/**阿拉伯语 */
        [LanguateText.粤语]: "yue", /**粤语 */
        [LanguateText.越南语]: "vi", /*越南语 */
        [LanguateText.德语]: "de", /**德语 */
        [LanguateText.意大利]: "it", /*意大利 */
    }

    constructor(){
        super();
        this.Init();
    }
    /**
     * 不支持专门去查询文字的语言
     * @returns 
     */
    public CanFindLanguage():boolean{
        return false;
    }
    /**
     * 支持自动识别语言进行翻译到目标语言
     * @returns 
     */
    public CanAutoTr():boolean{
        return true;
    }
    /**
     * 本为耗时操作，自动放入其他线程完成
     * @returns 
     */
    public Init(): void {
        if(this.isInit){
            return;
        }

        let hasErr = false;
        let ErrStr = "";
        let over = false;

        let _key:number;
        let _token:string;
        let _tokenTime:number;
        createThread(()=>{
            try{
                let ret_state  = http.Get("https://cn.bing.com/translator");
                if(ret_state?.status == 200){
                    const regex = /(?:var|let|const)\s+params_AbusePreventionHelper\s*=\s*\[\s*(\d+)\s*,\s*"([^"]+)"\s*,\s*(\d+)\s*\]\s*;?/;
                    const match = ret_state.body.match(regex);
                    if(!match){
                        //TODO: 失败
                        hasErr = true;
                        ErrStr = "微软翻译初始化,正则查询失败 " + ret_state.body.slice(0, 500);
                        over = true;
                        return;
                    }
                    _key = parseInt(match[1], 10);
                    _token = match[2];
                    _tokenTime = parseInt(match[3], 10);
                    if (isNaN(_key) || isNaN(_tokenTime) || _token.length < 5){
                        hasErr = true;
                        ErrStr = "提取的参数无效 " + `{ ${_key}, ${_token}, ${_tokenTime} }`;
                        over = true;
                        return;
                    }
                    over = true;
                    return;
                }else{
                    //TODO: 失败
                    hasErr = true;
                    ErrStr = "微软翻译初始化, 请求返回的结果 status:" + ret_state?.status || '无响应';
                    over = true;
                    return;
                }
            }catch(e:any){
                hasErr = true;
                ErrStr = "初始化代码抛出异常:" + e;
                over = true;
            }
        })
        ClientInstance.PushNextGameThreadWait({
            mainCall:()=>{
                if(hasErr){
                    logger.error("微软引擎:",ErrStr);
                }else{
                    this.key = _key;
                    this.token = _token;
                    this.tokenTime = _tokenTime;
                    this.isInit = true;
                }
            },
            CheckCall:()=>{
                return over;
            }
        })
    }

    public getInternalLanguageCode(id: LanguateText):string|null{
        return MicrofostTr.languageTabel[id]??null;
    }
    /**
     * （不要高频调用）根据 此引擎中的语言标识“en”转为number
     * @param lg 
     * @returns 如果为null 表示不存在
     */
    public static getCodeByInternalLanguage(lg: string):LanguateText|null{
        let ret:LanguateText | null = null;
        Object.entries(MicrofostTr.languageTabel).forEach(([key, value]) => {
            if(value == lg){
                ret = Number(key);
                return;
            }
        });
        return ret;
    }

    public OnlineTr(str:string/*要翻译的文本*/,from:LanguateText, to: LanguateText,call:(str:string,sourceLg:LanguateText,hasErr:boolean,errstr:string)=>void):void{
        if(!this.isInit){
            return call("",0,true,"此翻译引擎尚未初始化")
        }
        let over : boolean = false;
        let ret : http.Response|null = null;
        let ajaxErr:string|null = null;

        let fromlg: string|null = this.getInternalLanguageCode(from);
        let tolg :string|null = this.getInternalLanguageCode(to);
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
                    url: "https://cn.bing.com/ttranslatev3?IG=1E8C8539A9AF4A30B44FCBD1A0E1A5C5&IID=translator.5026",
                    method:'Post',
                    headers:{
                        'Content-Type': 'application/x-www-form-urlencoded',
                        'Origin': 'https://cn.bing.com',/*非必要 */
                        'Referer': 'https://cn.bing.com/translator?ref=TThis',
                    },
                    body:`fromLang=${fromlg}&to=${tolg}&text=${encodeURIComponent(str)}&token=${this.token}&key=${this.key}`,
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
                            call(str,0, true, "翻译调用异常:"+ajaxErr);
                        }else{
                            call(str,0, true, "ret == null");
                        }
                        return;
                    }
                    if(ret.status == 200){
                        if(ret.body.indexOf("[") != 0 /*&& ret.body.indexOf("{") != 0 */){  // 出错
                            call(str,0/*TODO:*/, true,"v[0]:'"+ret.body[0]+"' "+ ret.body);
                            return;
                        }else{
                            let v = JSON.parse(ret.body);
                            let Trtext :string = v[0]['translations'][0]['text'];
                            let _sourceLg = v[0]['detectedLanguage']['language'];
                            let lgcode = MicrofostTr.getCodeByInternalLanguage(_sourceLg)??0;
                            call(Trtext,lgcode, false, "");
                            return;
                        }
                    }else{
                        call(str,0, true, "ret.status"+ret.status);
                        return;
                    }
                }catch(e:any){
                    call(str,0, true, e + ((ret && ret.status == 200)?ret!.body:""));
                }
            },
            CheckCall:()=>{
                return over;
            }
        })
    }
}