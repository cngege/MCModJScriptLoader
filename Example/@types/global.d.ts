
/**
 * 加载时当前模块的状态
 */
declare const __模块__: 类型_公共模块;

declare interface 类型_公共模块 {
    /**
     * 是否启用 默认true
     */
    enable: boolean;
    /**
     * 模块名称
     */
    name: string;
    /**
     * 获取控制台窗口是否打开
     */
    JSConsoleWindow(): boolean;
    /**
     * 设置控制台窗口是否打开
     * @param enable 是否打开
     */
    JSConsoleWindow(enable: boolean): void;
}

declare interface ref_bool {
    bool: boolean;
}

declare interface ref_i {
    int: number;
}

declare interface ref_f {
    float: number;
}

declare interface vec_2 {
    x: number;
    y: number;
}

declare interface vec_3 {
    x: number;
    y: number;
    z: number;
}

declare interface vec_4 {
    x: number;
    y: number;
    z: number;
    w: number;
}

declare interface color_3 {
    r: number;
    g: number;
    b: number;
}

declare interface color_4 {
    r: number;
    g: number;
    b: number;
    a: number;
}

/**
 * 内存指针
 */
declare class NativePoint {
    constructor(ptr: number)
    constructor(ptr: number, size: number)
    constructor(ptr: number, ctype: number[])
    /**
     * 作为函数调用
     */
    call: (...args: any[])=> any;
    /**
     * 偏移处理(返回一个新对象,而不是this)
     * @param offset 偏移量
     */
    offset(offset  : Number) : NativePoint;
    /**
     * 返回一个经过偏移后的新地址(返回一个新对象,而不是this)
     * @param offset 偏移量
     */
    add(offset  : Number) : NativePoint;
    /**
     * 转为一个整数
     */
    toNumber() : number;
    /**
     * 争对call 设置参数类型
     * @param ctype 参数类型 第一个是返回值类型
     */
    setAgree(ctype : number[]) : NativePoint;
    /**
     * 对地址进行bool转换后设置一个bool值
     * @param v 
     */
    setbool(v : boolean) : void;
    /**
     * 对地址进行bool转换后读取一个bool值
     */
    getbool() : boolean;
    /**
     * 设置一个char类型的值, 影响一个字节
     * @param v 
     */
    setchar(v : Number) : void;
    /**
     * 获取一个char类型的值, 读取一个字节
     */
    getchar() : number;
    /**
     * 设置一个uchar类型的值, 影响一个字节
     * @param v 
     */
    setuchar(v : Number) : void;
    /**
     * 获取一个uchar类型的值, 读取一个字节
     */
    getuchar() : number;
    /**
     * 设置一个short类型的值, 影响两个字节
     * @param v 
     */
    setshort(v : Number) : void;
    /**
     * 获取一个short类型的值, 读取两个字节
     */
    getshort() : number;
    /**
     * 设置一个ushort类型的值, 影响两个字节
     * @param v 
     */
    setushort(v : Number) : void;
    /**
     * 获取一个ushort类型的值, 读取两个字节
     */
    getushort() : number;
    /**
     * 设置int类型的值, 影响四个字节
     * @param v 
     */
    setint(v : Number) : void;
    /**
     * 获取int类型的值, 读取四个字节
     */
    getint() : number;
    /**
     * 设置uint类型的值, 影响四个字节
     * @param v 
     */
    setuint(v : Number) : void;
    /**
     * 获取uint类型的值, 读取四个字节
     */
    getuint() : number;
    /**
     * 设置long类型的值, 影响四个字节
     * @param v 
     */
    setlong(v : Number) : void;
    /**
     * 读取long类型的值, 读取四个字节
     */
    getlong() : number;
    /**
     * 设置ulong类型的值, 影响四个字节
     * @param v 
     */
    setulong(v : Number) : void;
    /**
     * 读取ulong类型的值, 读取四个字节
     */
    getulong() : number;
    /**
     * 设置一个浮点数类型的值, 影响四个字节
     * @param v 
     */
    setfloat(v: number) : void;
    /**
     * 读取一个浮点数类型的值, 读取四个字节
     */
    getfloat() : number;
    /**
     * 设置一个双精度浮点数类型的值, 影响八个字节
     * @param v 
     */
    setdouble(v: number) : void;
    /**
     * 读取一个双精度浮点数类型的值, 读取八个字节
     */
    getdouble() : number;
    /**
     * 设置一个NativePoint类型的指针, 影响八个字节
     * @param v 
     */
    setpoint(v: NativePoint) : void;
    /**
     * 读取一个NativePoint类型的指针, 读取八个字节
     */
    getpoint(): NativePoint;
    /**
     * 指针强转字符串 *(std::string*)thi->m_ptr;
     */
    getstring() : string;
    /**
     * 对当前指针设置一个C++字符串(std::string)
     */
    setstring(str : string) : void;
    /**
     * 对当前指针填充数组, 请控制好长度,防止越界
     * @param bytes 请确保值范围在char范围内
     */
    setbytes(bytes: number[]): void
    /**
     * 指针强转C语言类型字符串 (const char*)thi->m_ptr;
     */
    getcstring(): string;
    /**
     * 复制到ArrayBuffer数组中,对其修改不影响原指针
     */    
    copyToArrayBuffer():ArrayBuffer;
    /**
     * 同指针映射到ArrayBuffer数组中,修改会影响源地址,注意只能在原对象生命周期内修改完成
     */
    toArrayBuffer():ArrayBuffer;
    /**
     * 使用buffer填充到指针中
     * @param array 
     * @param size  填充大小,不能大于 array长度 
     */
    fillArrayBuffer(array: Uint8Array, size?:number):void;
    /**
     * 设置内存页是否可以读写执行
     * @param size 修改的范围
     * @param status 状态值,具体值可以参考：https://learn.microsoft.com/zh-cn/windows/win32/Memory/memory-protection-constants
     * @returns 旧的内存页状态值,为null表示失败
     */
    setVirtualProtect(size: number, status: number):number|null;
    /**
     * 当NativePoint对象被释放，内部析构函数调用时触发
     * @param callback 
     */
    onfree(callback: (ptr:number)=>void):void;
}

/**
 * 日志
 */
declare class spdlog{
    /**
     * 创建一个日志对象
     * @param name [可选]日志头名称
     */
    constructor(name?: string);
    /**
     * 打印普通日志
     * @param text 打印的日志文本 
     */
    info(...text: unknown[]): void;
    /**
     * 打印警告日志
     * @param text 打印的日志文本 
     */
    warn(...text: unknown[]): void;
    /**
     * 打印错误日志
     * @param text 打印的日志文本 
     */
    error(...text: unknown[]): void;
    /**
     * 打印debug日志
     * @param text 打印的日志文本 
     */
    debug(...text: unknown[]): void;

    trace(...text: unknown[]): void;
}
/**
 * hook相关类
 */
declare class HookBase{
    /**
     * 创建一个游戏函数hook
     * @param hookback hook后回调函数
     * @param ptr hook函数地址
     * @param ctype 参数列表
     */
    constructor(hookback : (...args: any[]) => any, ptr : number, ctype: number[])
    /**
     * 开启hook
     */
    hook(): void;
    /**
     * 关闭hook(注意调用线程)
     */
    unhook(): void;
    /**
     * 原函数
     */
    origin : NativePoint;
}

declare function createThread(call: ()=>void, sleep?:number):number
declare function thread_sleep(sleep?:number):void
declare function testDumpValue(all: Object):void

/**
 * 参数的类型
 */
declare enum NativeTypes {
  Void = 0,
  Bool = 1,
  Char = 2,
  UnsignedChar = 3,
  Short = 4,
  UnsignedShort = 5,
  Int = 6,
  UnsignedInt = 7,
  Long = 8,
  UnsignedLong = 9,
  LongLong = 10,
  UnsignedLongLong = 11,
  Float = 12,
  Double = 13,
  Pointer = 14
}