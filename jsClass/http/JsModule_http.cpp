#include "quickjs/quickjs.h"
#include "../JSManager.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "http/httplib.h"
#include "spdlog/spdlog.h"

#include <string>

/**
  let res = http.Get("https://www.baidu.com/index.html");
  res.
*/

static JSValue js_http_get_request(JSContext * ctx, JSValueConst this_val, int argc, JSValueConst * argv)
{
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "至少需要一个参数");
    }
    const char* url_c = JS_ToCString(ctx, argv[0]);
    std::string url = std::string(url_c);
    JS_FreeValue(ctx, argv[0]);
    JS_FreeCString(ctx, url_c);

    // 解析数据:
    bool ishttps = false;
    auto f1 = url.find("https://");
    if(f1 == 0) {
        ishttps = true;
    }
    if(f1 == std::string::npos) {
        f1 = 0;
    }
    std::string host = "";
    auto f2 = url.find("://");
    if(f2 == std::string::npos) {
        f2 = 0;
    }
    else {
        f2 = f2 + 3;
    }

    auto f3 = url.find("/", f2);
    std::string weibu = "/";
    if(f3 != std::string::npos) {
        weibu = url.substr(f3);
        host = url.substr(f2, f3 - f2);
    }
    else {
        host = url.substr(f2);
    }

    httplib::Client https(host);
    https.enable_server_certificate_verification(false);
    if(!https.is_valid()) {
        return JS_NULL;
    }
    https.set_follow_location(true);
    auto res = https.Get(weibu);
    if(!res) {
        return JS_NULL;
    }
    JSValue retJSV = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, retJSV, "body", JSTool::fromString(res->body));
    JS_SetPropertyStr(ctx, retJSV, "version", JSTool::fromString(res->version));
    JS_SetPropertyStr(ctx, retJSV, "status", JS_NewInt32(ctx, res->status));

    JSValue headersJSV = JS_NewObject(ctx);
    for(const auto& item : res->headers) {
        JS_SetPropertyStr(ctx, headersJSV, item.first.c_str(), JSTool::fromString(item.second));
    }
    JS_SetPropertyStr(ctx, retJSV, "headers", headersJSV);

    return retJSV;
}

static JSValue js_http_post_request(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "至少需要一个参数");
    }
    const char* url_c = JS_ToCString(ctx, argv[0]);
    std::string url = std::string(url_c);
    JS_FreeValue(ctx, argv[0]);
    JS_FreeCString(ctx, url_c);

    // 解析数据:
    bool ishttps = false;
    auto f1 = url.find("https://");
    if(f1 == 0) {
        ishttps = true;
    }
    if(f1 == std::string::npos) {
        f1 = 0;
    }
    std::string host = "";
    auto f2 = url.find("://");
    if(f2 == std::string::npos) {
        f2 = 0;
    }
    else {
        f2 = f2 + 3;
    }

    auto f3 = url.find("/", f2);
    std::string weibu = "/";
    if(f3 != std::string::npos) {
        weibu = url.substr(f3);
        host = url.substr(f2, f3 - f2);
    }
    else {
        host = url.substr(f2);
    }

    httplib::Client https(host);
    https.enable_server_certificate_verification(false);
    if(!https.is_valid()) {
        return JS_NULL;
    }
    https.set_follow_location(true);
    auto res = https.Post(weibu);
    if(!res) {
        return JS_NULL;
    }
    JSValue retJSV = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, retJSV, "body", JSTool::fromString(res->body));
    JS_SetPropertyStr(ctx, retJSV, "version", JSTool::fromString(res->version));
    JS_SetPropertyStr(ctx, retJSV, "status", JS_NewInt32(ctx, res->status));

    JSValue headersJSV = JS_NewObject(ctx);
    for(const auto& item : res->headers) {
        JS_SetPropertyStr(ctx, headersJSV, item.first.c_str(), JSTool::fromString(item.second));
    }
    JS_SetPropertyStr(ctx, retJSV, "headers", headersJSV);

    return retJSV;
}

static JSValue js_http_ajax_request(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "至少需要一个参数");
    }

    JSValue JUrl = JS_GetPropertyStr(ctx, argv[0], "url");
    auto isUrl = JSTool::toString(JUrl);
    if(!isUrl) return JS_ThrowTypeError(ctx, "url属性不存在");
    std::string url = *isUrl;

    JSValue JMethod = JS_GetPropertyStr(ctx, argv[0], "method");
    auto hasMethod = JSTool::toString(JMethod);
    std::string method = "Get";
    if(hasMethod) {
        method = *hasMethod;
    }

    // 拿到headers属性{}
    JSValue JHeaders = JS_GetPropertyStr(ctx, argv[0], "headers");
    std::optional<std::vector<std::pair<std::string, std::string>>> headers{};
    headers = JSTool::getObjKV(JHeaders);


    //拿到 success 属性回调函数
    JSValue JSuccessCall = JS_GetPropertyStr(ctx, argv[0], "success");
    bool hasSuccessCall = JS_IsFunction(ctx, JSuccessCall);
    //拿到download属性回调函数
    JSValue JDownloadCall = JS_GetPropertyStr(ctx, argv[0], "download");
    bool hasDownloadCall = JS_IsFunction(ctx, JDownloadCall);

    //followlocation属性bool值 301状态是否自动跳转
    JSValue Jfollowlocation = JS_GetPropertyStr(ctx, argv[0], "followlocation");
    JSValue Jtimeout = JS_GetPropertyStr(ctx, argv[0], "timeout");
    auto isTimeout = JSTool::toInt64(Jtimeout);

    // 解析数据:
    bool ishttps = false;
    auto f1 = url.find("https://");
    if(f1 == 0) {
        ishttps = true;
    }
    if(f1 == std::string::npos) {
        f1 = 0;
    }
    std::string host = "";
    auto f2 = url.find("://");
    if(f2 == std::string::npos) {
        f2 = 0;
    }
    else {
        f2 = f2 + 3;
    }

    auto f3 = url.find("/", f2);
    std::string weibu = "/";
    if(f3 != std::string::npos) {
        weibu = url.substr(f3);
        host = url.substr(f2, f3 - f2);
    }
    else {
        host = url.substr(f2);
    }

    httplib::Client https(host);
    https.enable_server_certificate_verification(false);
    if(!https.is_valid()) {
        return JS_NULL;
    }
    if(JS_IsBool(Jfollowlocation) && JS_ToBool(ctx, Jfollowlocation)) {
        https.set_follow_location(true);
    }
    if(isTimeout) {
        https.set_connection_timeout(isTimeout.value() * 1000);
    }
    
    httplib::Headers Clientheaders = {};
    if(headers) {
        for(auto& item : *headers) {
            Clientheaders.emplace(item.first, item.second);
        }
    }
    httplib::Params params = {};
    // 拿到请求数据
    JSValue Jbody = JS_GetPropertyStr(ctx, argv[0], "body");

    std::string body;
    if(JS_IsObject(Jbody)) {// 检查是否为对象
        auto v = JS_JSONStringify(ctx, Jbody, JS_UNDEFINED, JS_UNDEFINED);
        if(JS_IsException(v)) {
            return v;
        }
        auto str = JSTool::toString(v);
        if(!str) return JS_ThrowTypeError(ctx, "请求参数无法序列化为字符串");
        body = *str;
    }
    else {
        if(JS_IsString(Jbody)) {
            auto str = JSTool::toString(Jbody);
            if(!str) return JS_ThrowTypeError(ctx, "请求参数无法序列化为字符串");
            body = *str;
        }
    }
    // 如果有下载监听 则创建下载回调
    httplib::Progress downloadcall = nullptr;
    if(hasDownloadCall) {
        downloadcall = [&](uint64_t len, uint64_t total) {
            JSValue argv2[] = { JS_NewInt64(ctx, (int64_t)len), JS_NewInt64(ctx, (int64_t)total) };
            JSValue Jret = JS_Call(ctx, JDownloadCall, this_val, 2, argv2);
            if(!JS_IsBool(Jret)) {
                return true;
            }
            else {
                return (bool)JS_ToBool(ctx, Jret);
            }
        };
    }
    httplib::Result res;
    if(method == "Post") {
        std::string content_type = "application/json";
        auto it = Clientheaders.find(std::string("content-type"));
        if(it != Clientheaders.end()) {
            content_type = it->second;
        }
        
        res = https.Post(weibu, Clientheaders, body, content_type, downloadcall);
    }
    else {
        
        if(!params.empty()) {
            //// 额外指定了参数
            //auto wenhao = weibu.find("?");
            //weibu += (wenhao == std::string::npos) ? "?" : "&";
            //for(auto& item : *data) {
            //    weibu += item.first;
            //    weibu += "=";
            //    weibu += item.second;
            //}
            spdlog::warn("Option.data 参数仅用于Post, Get请用?xx==xx格式");
        }
        res = https.Get(weibu, Clientheaders, downloadcall);
    }
    
    if(!res) {
        if(hasSuccessCall) {
            return JS_UNDEFINED;
        }
        return JS_NULL;
    }

    JSValue retJSV = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, retJSV, "body", JSTool::fromString(res->body));
    JS_SetPropertyStr(ctx, retJSV, "version", JSTool::fromString(res->version));
    JS_SetPropertyStr(ctx, retJSV, "status", JS_NewInt32(ctx, res->status));

    JSValue headersJSV = JS_NewObject(ctx);
    for(const auto& item : res->headers) {
        JS_SetPropertyStr(ctx, headersJSV, item.first.c_str(), JSTool::fromString(item.second));
    }
    JS_SetPropertyStr(ctx, retJSV, "headers", headersJSV);

    if(hasSuccessCall) {
        JSValue canshu[] = { retJSV };
        JS_Call(ctx, JSuccessCall, this_val, 1, canshu);
        return JS_UNDEFINED;
    }
    return retJSV;
}

static const JSCFunctionListEntry js_http_funcs[] = {
    JS_CFUNC_DEF2("Get", 1, js_http_get_request),
    JS_CFUNC_DEF2("Post", 1, js_http_post_request),
    JS_CFUNC_DEF2("Ajax", 1, js_http_ajax_request),
};

static int js_http_init(JSContext* ctx, JSModuleDef* m)
{
   return JS_SetModuleExportList(ctx, m, js_http_funcs, _countof(js_http_funcs));
}

JSModuleDef* js_init_module_http(JSContext* ctx, const char* module_name)
{
   JSModuleDef * m = JS_NewCModule(ctx, module_name, js_http_init);
   if(!m)
      return NULL;
   JS_AddModuleExportList(ctx, m, js_http_funcs, _countof(js_http_funcs));
   return m;
}