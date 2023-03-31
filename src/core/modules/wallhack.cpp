#include "wallhack.h"

#include "../core.h"
#include "../draw_list.h"
#include <iostream>

void Wallhack::tick(float delta_time) {
}

void Wallhack::draw() {
    for (auto entity : core::entities) {
        if (!entity.valid()) continue;
        vec4 box;

        if (utils::get_box_coords(entity, box)) {
            draw_list.add_rect_corners(box.xy(), box.zw(), 0.6f, 0.55f, vec3(0.8f,0,0));
        }
    }
}

void Wallhack::init() {
}