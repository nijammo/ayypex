#pragma once
#include "imgui.h"
#include "vmt_hook.h"
#include "../core/core.h"
#include "createmove.h"
#include <exception>
#include <iostream>

inline void onFirstFrame() {
    init();
    core::init();
}

inline void onFrame(ImVec2 window_size, bool) {
    core::draw();
}