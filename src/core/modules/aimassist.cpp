#include "aimassist.h"

#include "../core.h"
#include "../draw_list.h"
#include "../prediction.hpp"
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

using prediction::prediction_ctx;

void AimAssist::tick(float delta_time) {
    auto weapon = core::local_player.get_held_weapon();

    if (!weapon.valid())
        return;

    target.invalidate();

    float closest_distance = std::numeric_limits<float>::max();
    for (auto entity : core::entities) {
        if (!entity.valid())
            continue;

        Player player(entity.get_address(), entity.get_index());

        float distance = math::get_view_delta(core::local_player.get_camera_position(), player.get_bone(), core::local_player.get_angles()).magnitude();

        if (distance > 360) {
            distance -= 360;
        }

        if (distance > 10)
            continue;

        if (distance < closest_distance) {
            closest_distance = distance;
            target.set_address(player.get_address(), player.get_index());
        }
    }


    if (!target.valid())
        return;
    
    float distance = math::get_view_delta(core::local_player.get_camera_position(), target.get_bone(), core::local_player.get_angles()).magnitude();

    if (!core::local_player.is_zooming())
        return;
    
    bool is_teammate = target.get_team() == core::local_player.get_team() ? 1 : 0;
    if (!target.is_visible() || is_teammate) {
        return;
    }

    prediction_ctx ctx;

    ctx.gravity = weapon.get_gravity();
    ctx.target_velocity = target.get_velocity();
    ctx.target_position = target.get_bone(1);
    ctx.launch_position = core::local_player.get_camera_position();
    ctx.launch_velocity = weapon.get_speed();
    prediction::predict_trajectory(ctx);

    auto get_fov_scale = [&weapon]() {
        if (core::local_player.is_zooming()) {
            if (weapon.get_zoom_fov() != 0.0f && weapon.get_zoom_fov() != 1.0f) {
                return weapon.get_zoom_fov() / 90.0f;
            }
        }
        return 1.0f;
    };

    if (ctx.success) {
        float delta = (core::local_player.get_angles() - ctx.angles).magnitude();
        float fov_scale = get_fov_scale();
        if (delta < 0.3f * fov_scale) {
            return;
        }

        float aim_strength = 2.2f;

        vec3 sway = core::local_player.get_sway() * 1.0f;
        
        float speed = logf(aim_strength + delta / (fov_scale * fov_scale) * aim_strength) * aim_strength + aim_strength;

        vec3 delta_angles = ctx.angles - core::local_player.get_angles();
        vec3 angles = (delta_angles + sway)* speed * delta_time;

        vec3 current_angles = core::local_player.get_angles();
        core::local_player.set_angles(current_angles + angles);
    }
}

Player AimAssist::target(0, 0);

void AimAssist::draw() {
    auto weapon = core::local_player.get_held_weapon();

    if (!weapon.valid())
        return;
}

void AimAssist::init() {}