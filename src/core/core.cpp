#include "core.h"

#include "draw_list.h"
#include "mem.h"
#include "modules/aimassist.h"
#include "modules/glow.h"
#include "modules/rcs.h"
#include "modules/wallhack.h"
#include "offsets.h"
#include "sdk.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <string>
#include <sys/types.h>
#include <vector>
#include <iostream>

namespace core {
    std::vector<Module*> modules;
    std::vector<Entity> entities;
    std::vector<Entity> items;
    Player local_player;
    std::chrono::high_resolution_clock timer;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_tick;

    void update_entities() {
        short local_player_handle = mem::read<short>(BASE + offsets::local_player_handle);
        if (local_player_handle == -1) return;
        if (mem::read<uintptr_t>(BASE + offsets::entity_list) == 0) return;

        entities.clear();
        local_player.invalidate();

        for (int i = 0; i < NUM_ENTS; i++) {
            Entity entity = Entity::get_by_id(i);
            if (entity.valid()) {
                if (i == local_player_handle) {
                    local_player.set_address(entity.get_address(), i);
                    continue;
                }

                if (entity.get_address() == local_player.get_address()) continue;

                uintptr_t signifier_ptr = mem::read<uintptr_t>(entity.get_address() + offsets::entity::name);
                if (!signifier_ptr) continue;

                uintptr_t signifier = mem::read<uintptr_t>(signifier_ptr);

                if (signifier == PLAYER || signifier == NPC_DUMMIE) {
                    entities.push_back(entity);
                }
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

    #pragma pack(push,1)
    struct weaponsetting_t {
        char* name;
        void* unk1;
        char* description;
        char type;
        char unk2;
        ushort index;
        ushort offset;
        char null1[2];
    };
    #pragma pack(pop)

    void init() {
        uintptr_t weaponsettings = 0x14B364BC0;
        ptrdiff_t size = 0x20;

        for (int i = 0; i < 1152; i++) {
            weaponsetting_t setting = mem::read<weaponsetting_t>(weaponsettings + size * i);

            std::cout << "Setting: " << setting.name << std::endl;
            std::cout << "Offset: " << std::hex << setting.offset << std::endl << std::endl;
        }

        modules.push_back(new Wallhack());
        //modules.push_back(new AimAssist());
        modules.push_back(new Glow());
        modules.push_back(new RecoilControl());

        for (auto& module : modules) {
            module->init();
        }
    }
}