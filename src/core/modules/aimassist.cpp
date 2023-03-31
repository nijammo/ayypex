#include "aimassist.h"

#include "../core.h"
#include "../draw_list.h"
#include "../prediction.hpp"
#include <cfloat>
#include <iostream>
#include <string>

using prediction::prediction_ctx;

void AimAssist::tick(float delta_time) {
    auto weapon = core::local_player.get_held_weapon();

    if (!weapon.valid()) return;

    target.invalidate();

    float closest_distance = std::numeric_limits<float>::max();
    for (auto entity : core::entities) {
        if (!entity.valid()) continue;

        Player player(entity.get_address());

        float distance = math::get_view_delta(core::local_player.get_camera_position(), player.get_bone(), core::local_player.get_angles()).magnitude();

        if (distance > 10) continue;

        if (distance < closest_distance) {
            closest_distance = distance;
            target.set_address(player.get_address());
        }
    }

    if (!target.valid()) return;
    if (!core::local_player.is_zooming()) return;

    prediction_ctx ctx;
    ctx.launch_position = core::local_player.get_camera_position();
    ctx.launch_velocity = weapon.get_speed();
    ctx.gravity = weapon.get_gravity();
    ctx.target_position = target.get_bone();
    ctx.target_velocity = target.get_velocity();

    if (prediction::predict_trajectory(ctx)) {
        ctx.angles = ctx.angles + core::local_player.get_sway_angles();

        vec3 delta = (ctx.angles - (core::local_player.get_angles()));

        float yaw = yaw_controller.update(delta.x, delta_time);
        float pitch = pitch_controller.update(delta.y, delta_time);

        vec3 current_angles = core::local_player.get_angles();

        current_angles.x += yaw * delta_time;
        current_angles.y += pitch * delta_time;

        core::local_player.set_angles(current_angles);
    }
}

void AimAssist::draw() {
    auto weapon = core::local_player.get_held_weapon();

    if (!weapon.valid()) return;


}

void AimAssist::init() {
}