#include "rcs.h"

#include "../core.h"
#include <iostream>

void RecoilControl::tick(float delta_time) {
    vec3 angles = core::local_player.get_angles();
    vec3 punch = core::local_player.get_recoil();
    vec3 current_punch = (old_angles - punch);
    current_punch.x *= 0.2f;
    current_punch.y *= 0.4f;

    core::local_player.set_angles(angles + current_punch);
    old_angles = punch;
}

void RecoilControl::draw() {
}

void RecoilControl::init() {
}