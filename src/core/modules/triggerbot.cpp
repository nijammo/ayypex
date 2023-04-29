#include "triggerbot.h"

#include "../core.h"
#include "../draw_list.h"
#include <iostream>

void Triggerbot::tick(float delta_time) {
    auto current_time = Game::get_current_time();
    auto last_crosshair_time = mem::read<float>(core::local_player.get_address() + 0x1A80);

    if (current_time - last_crosshair_time < 0.1f) {
        core::local_player.shoot();
    }
}

void Triggerbot::draw() {
}

void Triggerbot::init() {
}