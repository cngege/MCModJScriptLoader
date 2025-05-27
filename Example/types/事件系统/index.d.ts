declare module "事件系统" {
  /**
   * 监听一个事件
   * @param sig 
   * @param callback 回调函数在出现异常的时候将会被 移除, 如果希望移除，则应使用try包裹
   */
  export function 监听事件(sig: string, callback : (...args: any[]) => boolean | void): void;
  /**
   * 从事件队列中手动移除一个回调事件
   * @param sig 
   * @param callback 
   */
  export function 移除事件(sig: string, callback : (...args: any[]) => boolean | void): void;
  /**
   * 广播/触发 一个类型事件队列
   * @param sig 
   * @param callback 
   */
  export function 事件广播(sig: string, callback : (ret : boolean) => void): void;
  export function 事件广播(sig: string, callback : (ret : boolean) => void, ...args: any[]): void;
}