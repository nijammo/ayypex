#include "core.h"

#include "draw_list.h"
#include "mem.h"
#include "modules/aimassist.h"
#include "modules/wallhack.h"
#include "offsets.h"
#include "sdk.h"
#include <cstdint>
#include <cstring>
#include <exception>
#include <vector>
#include <iostream>

namespace core {
    std::vector<Module*> modules;
    std::vector<Entity> entities;
    Player local_player;
    std::chrono::high_resolution_clock timer;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_tick;

    void update_entities() {
        // If mutex is locked, return
        //if (update_mutex.try_lock() == false) return;

        //std::lock_guard<std::mutex> lock(update_mutex);

        short local_player_handle = mem::read<short>(BASE + offsets::local_player_handle);
        if (local_player_handle == -1) return;
        if (!mem::read<uintptr_t>(BASE + offsets::entity_list)) return;

        entities.clear();
        local_player.invalidate();

        for (int i = 0; i < NUM_ENTS; i++) {
            Entity entity = Entity::get_by_id(i);
            if (entity.valid()) {
                if (i == local_player_handle) {
                    local_player.set_address(entity.get_address());
                    continue;
                }

                if (entity.get_address() == local_player.get_address()) continue;

                uintptr_t signifier_ptr = mem::read<uintptr_t>(entity.get_address() + offsets::entity::name);
                if (!signifier_ptr) continue;

                uintptr_t signifier = mem::read<uintptr_t>(signifier_ptr);

                if (signifier == PLAYER || signifier == NPC_DUMMIE)
                    entities.push_back(entity);
            }
        }
    }

    void tick(float delta_time) {
        update_entities();

        if (!local_player.valid()) return;

        for (auto& module : modules) {
            if (module->active) {
                module->tick(delta_time);
            }
        }
    }

    void draw() {

        auto now = timer.now();
        float delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_tick).count();
        core::tick(delta_time);
        last_tick = now;

        tick(delta_time);

        if (!local_player.valid()) return;

        for (auto& module : modules) {
            if (module->active) {
                module->draw();
            }
        }
    }

    void init() {
        modules.push_back(new Wallhack());
        modules.push_back(new AimAssist());

        for (auto& module : modules) {
            module->init();
        }
    }
}