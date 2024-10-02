#pragma once
#ifndef JSCLASS_EVENTSYSTEM_JSMODULEEVENTSYSTEM_H
#define JSCLASS_EVENTSYSTEM_JSMODULEEVENTSYSTEM_H

#include <string>
#include "quickjs/quickjs.h"
#include <functional>

// 二进制程序内部广播事件
void NativeBroadcastEvent(const std::string& name, JSValueConst this_val = JS_UNDEFINED, int argc = 0, JSValueConst* argv = nullptr, std::function<void(bool)> callback = nullptr);
// 二进制程序内部监听事件
JSValue NativeListenEvent(const std::string& eventname, JSCFunction jsfun, const std::string& funname);
// 二进制移除程序内部监听事件
void NativeRemoveEvent(const std::string& eventname, JSValue jsfun);
#endif // !JSCLASS_EVENTSYSTEM_JSMODULEEVENTSYSTEM_H
