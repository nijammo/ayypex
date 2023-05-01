#include "rcs.h"

#include "../core.h"
#include <iostream>
#include "../configuration.h"

void RecoilControl::tick(float delta_time) {
    vec3 angles = core::local_player.get_angles();
    vec3 punch = core::local_player.get_recoil();
    vec3 current_punch = (old_angles - punch);
    current_punch.x *= configuration::get_or_default<float>("recoil_control/horizontal_correction_strength", 0.7f);
    current_punch.y *= configuration::get_or_default<float>("recoil_control/vertical_correction_strength", 0.5f);

    core::local_player.set_angles(angles + current_punch);
    old_angles = punch;
}

void RecoilControl::draw() {
}

void RecoilControl::init() {
}