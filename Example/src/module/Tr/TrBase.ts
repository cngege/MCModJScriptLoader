
export enum LanguateText{
    不支持 = 0,
    自动检查 = 1,
    简体中文 =2,/**简体中文 */
    英语 =3,/**英语 */
    日语 =4,/**日语 */
    韩语 =5,/**韩语 */
    俄语 =6,/**俄语 */
    法语 =7, /**法语 */
    西班牙 =8, /**西班牙语 */
    阿拉伯 =9, /**阿拉伯语 */
    粤语 =10, /**粤语 */
    越南语 =11, /*越南语 */
    德语 =12,/* 德语 */
    意大利 =13 /*意大利 */
}

export class TrLanguage{
    private static isinit = false;
    private static isToLgText = false;

    /**
     * 字符串id之间的映射
     */
    private static LanguageTextArray : string[]=[];

    public static Init(){
        // 将Table 中的字符串->num映射转到LanguateText
        if(!this.isToLgText){
            const entries: Array<[key: string, value: LanguateText]> =  Object.entries(LanguateText).filter( ([key, value]) => typeof value === "number" ) as Array<[string, LanguateText]>;
            entries.sort((a, b) => a[1] - b[1]);
            this.LanguageTextArray = entries.map(([key]) => key);
            this.isToLgText = true;
        }
    }

    /**
     * 获取本地化语言字符串,比如 1 => 简体中文
     * @returns 
     */
    public static getLanguageTextByCode(code:number):string|null{
        let text = LanguateText[code];
        return text??null;
    }

    /**
     * 获取一个由语种key组成的字符串数组，便于在ImGui下拉框中轮询选择
     * @returns 
     */
    public static getLanguageTextTabel(){
        return this.LanguageTextArray;
    }
}

TrLanguage.Init();

export default class TrBase{
    
    /**
     * 是否初始化了（可能初始化失败）
     */
    protected isInit : boolean = false;

    constructor(){

    }

    /**
     * 将TrLanguage 中的语言id转为 对于翻译可以识别的代表语言的文本或数字 比如0=>auto-detect 1=> zh-Hans
     * @param id 
     * @returns 
     */
    public getInternalLanguageCode(id: LanguateText):string|number|null{
        return null;
    }

    public IsInit(){
        return this.isInit;
    }
    public Init(): void{}

    /**
     * 是否可以更具一段文字进行查询其语种信息
     * @returns 
     */
    public CanFindLanguage():boolean{
        return false;
    }
    /**
     * 是否支持自动识别原语种进行翻译
     * @returns 
     */
    public CanAutoTr():boolean{
        return false;
    }

    /**
     * 在线查询文本的语言
     * @param str 要查询的文本
     * @param call 查询结果回调
     */
    public OnlineCheckLanguage(str:string, call:(lg:LanguateText, haserr:boolean, err:string)=>void){
        call(0, true, "基类回调, 该语言引擎不支持");
    }
    /**
     * 翻译接口 等待被继承
     * @param str 
     * @param from 
     * @param to 
     * @param call 
     */
    public OnlineTr(str:string/*要翻译的文本*/,from:LanguateText, to: LanguateText,call:(str:string,sourceLg:LanguateText,hasErr:boolean,errstr:string)=>void):void{
        call("", 0, true, "基类回调, 该语言引擎不支持");
    }
}

export function isChinese(text:string, threshold = 0.5, maxCheck = 100):Boolean {
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