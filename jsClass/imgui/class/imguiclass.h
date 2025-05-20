#pragma once
#include "../../JSManager.h"

namespace JSForegroundDrawList {
    JSClassID getForegroundDrawListID();
}

namespace JSImGuiIO {
    JSClassID getImGuiIOID();
}

void imguiclass_init();
