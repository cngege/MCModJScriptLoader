#include "JsModule_http.h"
#include "quickjs/quickjs.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "http/httplib.h"

#include <string>

static JSValue js_http_get_request(JSContext * ctx, JSValueConst this_val, int argc, JSValueConst * argv)
{
    const char* url = JS_ToCString(ctx, argv[0]);
    JSValueConst func = argv[1];
    int sync = JS_ToBool(ctx, argv[2]);
    if(!JS_IsFunction(ctx, func)) {
        printf("not a function\n");
        return JS_EXCEPTION;

    }
    JS_FreeValue(ctx, argv[0]);
    //http_ctx_t * data = calloc(1, sizeof(http_ctx_t));
    //data->url = url;
    //data->ctx = ctx;
    //data->func = JS_DupValue(ctx, func);
    if(sync) {
        printf("c sync req\n");
        //curl_main(url, data);

    }
    else {
        printf("c async req\n");
        //data->handle.data = data;
        //uv_timer_init(loop, &data->handle);
        //uv_timer_start(&data->handle, js_http_get_request_async, 0, 0);

    }
    return JS_UNDEFINED;
}

static const JSCFunctionListEntry js_http_funcs[] = {
   //JS_CFUNC_DEF("Get", 1, js_http_get_request),
    { "Get", JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, 0, { .func = { 1, JS_CFUNC_generic,{ .generic = js_http_get_request } } } },
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