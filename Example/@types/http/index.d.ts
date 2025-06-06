declare module "http" {
  export interface Options {
    url: string;
    method?: "Get" | "Post";
    data?: Record<string, string>;// TODO
    headers?: Record<string, string>;//请求头
    body?: string | Record<string, any>;// 请求参数
    timeout?: number;                   // 单位ms
    followlocation?: boolean;
    download? : (len : number, total : number) => boolean;
    success? : (res : Response)=>void;
    error? :(err:string)=>void;
  }

  export interface Response {
    body : string;
    version : string;
    status : number;
    headers : Record<string, string>;
  }

  export function Get(url : string): Response | null;
  export function Post(url : string): Response | null;
  export function Ajax(opt : Options): Response | null;
}