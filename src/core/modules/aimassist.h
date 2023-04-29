#pragma once

#include "../module.h"
#include "../sdk.h"
#include "../pid.h"

class AimAssist : public Module {
    public:
    static Player target;

    // Randomly chosen values
    PID yaw_controller = PID(14.1, -5.8, 0.2, -15.0, 15.0);
    PID pitch_controller = PID(12.5, -5.8, 0.2, -15.0, 15.0);

    AimAssist() : Module() {
        active = true;
    }
    
    static void silent_aim();
    static void before_createmove();
    static void after_createmove();
    void tick(float delta_time) override;
    void draw() override;
    void init() override;
};