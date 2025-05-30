
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
    /**
     * 是否启用 默认true
     */
    call: (...args: any[])=> any;
    /**
     * 偏移处理(返回一个新对象,而不是this)
     * @param offset 偏移量
     */
    offset(offset  : Number) : NativePoint;
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
     * 设置一个short类型的值, 影响两个字节
     * @param v 
     */
    setshort(v : Number) : void;
    /**
     * 获取一个short类型的值, 读取两个字节
     */
    getshort() : number;
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
     * 设置long类型的值, 影响四个字节
     * @param v 
     */
    setlong(v : Number) : void;
    /**
     * 读取long类型的值, 读取四个字节
     */
    getlong() : number;
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
     * 指针强转C语言类型字符串 (const char*)thi->m_ptr;
     */
    getcstring(): string;
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