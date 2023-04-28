#pragma once

#include "../module.h"
#include "../math.h"

class RecoilControl : public Module {
    public:
    RecoilControl() : Module() {
        active = true;
    }

    vec3 old_angles = vec3(0,0,0);
    
    void tick(float delta_time) override;
    void draw() override;
    void init() override;
};