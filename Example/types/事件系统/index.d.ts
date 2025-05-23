declare module "事件系统" {
  // 函数签名
  export function 监听事件(sig: string, callback : (...args: any[]) => boolean | void): void;
  export function 移除事件(sig: string, callback : (...args: any[]) => boolean | void): void;
  export function 事件广播(sig: string, callback : (ret : boolean) => void): void;
  export function 事件广播(sig: string, callback : (ret : boolean) => void, ...args: any[]): void;
}