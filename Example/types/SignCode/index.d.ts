declare module "SignCode" {
  // 函数签名
  export default class SignCode{
    /**
     * 特征码查询工具
     * @param title 此次查找实例的标题
     */
    constructor(title : string);
    /**
     * 特征码查询工具
     * @param title 此次查找实例的标题
     * @param printfail 是否打印失败信息(默认 true)
     */
    constructor(title : string, printfail : boolean);
    /**
     * 特征码组是否查询成功
     */
    isOK() : boolean;
    /**
     * 获取特征码最终查询到的有效地址
     */
    get() : number;
    /**
     * 成功查询到的特征码
     */
    ValidSign() : string;
    /**
     * 有效指针地址(未处理过的)
     */
    ValidPtr() : number;
    /**
     * 添加一处特征码查询
     * @param signstr 特征码字符串
     */
    AddSign(signstr : string) : void;
    /**
     * 添加一处特征码查询
     * @param signstr 特征码字符串
     * @param callback 查到之后的回调, 用于处理查询后的地址
     */
    AddSign(signstr : string, callback : (v : number)=>number) : void;
    /**
     * 添加一处争对call的特征码查询
     * @param signstr 特征码字符串
     */
    AddSignCall(signstr : string) : void;
    /**
     * 添加一处争对call的特征码查询
     * @param signstr 特征码字符串
     * @param offset 对查到的地址进行偏移处理
     */
    AddSignCall(signstr : string, offset : number) : void;
    /**
     * 添加一处争对call的特征码查询
     * @param signstr 特征码字符串
     * @param offset 对查到的地址进行偏移处理
     * @param callback 对上出处理后的结果进行回调函数处理
     */
    AddSignCall(signstr : string, offset : number, callback : (v : number)=>number) : void;
  }
}