#include "glow.h"

#include "../core.h"
#include "../draw_list.h"
#include <iostream>

void Glow::tick(float delta_time) {
}

void Glow::draw() {
    for (auto entity : core::entities) {
        if (!entity.valid()) continue;

        bool is_teammate = entity.get_team() == core::local_player.get_team() ? 1 : 0;

        vec3 color = is_teammate ? vec3(0,10.0f,0) : vec3(15.0f,0.0f,0);

        entity.enable_glow();
        if (entity.is_visible() && !is_teammate) {
            entity.set_glow_color(vec3(0,0,15.0f));
        } else { 
            entity.set_glow_color(color); 
        }
    }
}

void Glow::init() {
}