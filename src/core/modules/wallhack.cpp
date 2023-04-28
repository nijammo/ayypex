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
            bool is_teammate = entity.get_team() == core::local_player.get_team() ? 1 : 0;

            draw_list.add_rect_corners(box.xy(), box.zw(), 0.6f, 0.55f, is_teammate ? vec3(0,0.8f,0) : vec3(0.8f,0,0));
            
            float ratio = (entity.get_health() + entity.get_shield()) / (entity.get_max_health() + entity.get_max_shield());
            float delta = box.y - box.w;

            auto color_lerp = [](float t, vec3 a, vec3 b) -> vec3 {
                return a + (b - a) * t;
            };

            vec3 full_health_color = vec3(0,1,0);
            vec3 no_health_color = vec3(1,0,0);

            vec3 health_color = color_lerp(ratio, no_health_color, full_health_color);

            draw_list.add_rect_filled(vec2(box.x - 5, box.w + delta * ratio), vec2(box.x - 3, box.w), health_color);
            draw_list.add_rect_filled(vec2(box.x - 6, box.w + delta * ratio - 1), vec2(box.x - 2, box.w + 1), vec4(0,0,0,0.5f));
        }
    }
}

void Wallhack::init() {
}