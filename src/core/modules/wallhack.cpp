#include "wallhack.h"

#include "../core.h"
#include "../draw_list.h"
#include <iostream>
#include "../configuration.h"

void Wallhack::tick(float delta_time) {
}

void Wallhack::draw() {
    for (auto entity : core::entities) {
        if (!entity.valid()) continue;
        vec4 box;

        if (utils::get_box_coords(entity, box)) {
            bool is_teammate = entity.get_team() == core::local_player.get_team() ? 1 : 0;

            vec3 enemy_color = {
                configuration::get_or_default<float>("wallhack/enemy_color_r", 1.0f),
                configuration::get_or_default<float>("wallhack/enemy_color_g", 0.0f),
                configuration::get_or_default<float>("wallhack/enemy_color_b", 0.0f)
            };
            
            vec3 friendly_color = {
                configuration::get_or_default<float>("wallhack/friendly_color_r", 0.0f),
                configuration::get_or_default<float>("wallhack/friendly_color_g", 1.0f),
                configuration::get_or_default<float>("wallhack/friendly_color_b", 0.0f)
            };

            vec3 color = is_teammate ? friendly_color : enemy_color;

            draw_list.add_rect_corners(box.xy(), box.zw(), 0.6f, 0.55f, color);
            
            float ratio = (entity.get_health() + entity.get_shield()) / (entity.get_max_health() + entity.get_max_shield());
            float delta = box.y - box.w;

            if (!is_teammate) 
                draw_list.add_rect_filled(vec2(box.x, box.w + delta * ratio), box.zw(), vec4(0.8f,0,0,0.45f));
        }
    }
}

void Wallhack::init() {
}