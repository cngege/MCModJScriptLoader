declare module "http" {
  export interface Options {
    url: string;
    method?: "Get" | "Post";
    headers?: Record<string, string>;   // TODO
    body?: string | Record<string, any>;// TODO
    timeout?: number;
    followlocation?: boolean;
    download? : (len : number, total : number) => boolean;
    success? : (res : Response)=>void;
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