#include "aimassist.h"

#include "../core.h"
#include "../draw_list.h"
#include "../prediction.hpp"
#include <cfloat>
#include <chrono>
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

        if (distance > 10)
            continue;

        if (distance < closest_distance) {
            closest_distance = distance;
            target.set_address(player.get_address(), player.get_index());
        }
    }

    //target_angles = math::calculate_angles(core::local_player.get_camera_position(), target.get_bone()).xy();
}

Player AimAssist::target(0, 0);

void AimAssist::draw() {
    auto weapon = core::local_player.get_held_weapon();

    if (!weapon.valid())
        return;
}

void AimAssist::init() {}