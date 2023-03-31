#pragma once

#include "../module.h"

class Wallhack : public Module {
    public:
    Wallhack() : Module() {
        active = true;
    }
    
    void tick(float delta_time) override;
    void draw() override;
    void init() override;
};