#pragma once
#include "imgui.h"
#include "vmt_hook.hpp"
#include "../core/core.h"
#include "createmove.hpp"
#include <exception>
#include <iostream>

inline void onFirstFrame() {
    init(); //createmove hook

    core::init();
}

inline void onFrame(ImVec2 window_size, bool) {
    core::draw();
}