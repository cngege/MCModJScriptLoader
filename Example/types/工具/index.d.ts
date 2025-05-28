declare module "工具" {
  // 函数签名
  export function 查找特征码(sig: string, module?: string): number;

  export function 二次查找特征码(BasePtr: number, Str: string, scan : number): number;
  /**
   * 获取游戏基址
   */
  export function 获取基址(): number;
  /**
   * 获取指定模块的基址
   * @param module 模块名称
   */
  export function 获取基址(module: string): number;
  export function 设置内存Bool值(ptr : number, value : boolean): void;
  export function 读取内存Bool值(ptr : number): boolean;
  export function 世界位置转屏幕位置(ptr : NativePoint, ori: vec_3, pos: vec_3, fov: vec_2, screenSize: vec_2): boolean;

}