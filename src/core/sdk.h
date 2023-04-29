#pragma once

#include "imgui.h"
#include "offsets.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <sys/types.h>
#include "math.h"
#include "mem.h"

#define NUM_ENTS 0x10000
#define IN_ATTACK 1
#define BASE 0x140000000

#define PLAYER 0x726579616c70
#define PROP_SURVIVAL 0x7275735f706f7270
#define NPC_DUMMIE 0x6d6d75645f63706e

struct CEntInfo {
        uintptr_t pEntity;
        int64_t SerialNumber;
        uintptr_t pPrev;
        uintptr_t pNext;
};

struct glow_mode {
    uint8_t border_mode, general_mode, border_size, unk;
};

class Entity {
    protected:
    uintptr_t address;
    int index;
    public:
    Entity() : address(0), index(0) {};
    Entity(uintptr_t address, int index) : address(address), index(0) {}

    uintptr_t get_address() {
        return address;
    }

    int get_index() {
        return index;
    }

    void set_address(uintptr_t addr, int index) {
        this->address = addr;
        this->index = index;
    }

    void invalidate() {
        address = 0;
    }

    float get_health() {
        return mem::read<int>(address + offsets::entity::health);
    }

    float get_shield() {
        return mem::read<int>(address + offsets::entity::shield);
    }

    float get_max_shield() {
        return mem::read<int>(address + offsets::entity::max_shield);
    }

    int get_script_id() {
        return mem::read<int>(address + offsets::entity::script_index);
    }

    int get_script_name() {
        return mem::read<int>(address + 0x690);
    }

    int get_team() {
        return mem::read<int>(address + offsets::entity::team_id);
    }

    void enable_glow() {
        int glow_enabled = mem::read<int>(address + 0x03c8);
        if (glow_enabled != 1) {
            mem::write<int>(address + 0x03c8, 1);
        }
        int glow_context = mem::read<int>(address + 0x03d0);
        if (glow_context != 2) {
            mem::write<int>(address + 0x03d0, 2);
        }

        const glow_mode default_mode = {101,102,49,75};
        int default_mode_int = *(int*)&default_mode;
        if (mem::read<int>(address + 0x02c4) != default_mode_int) {
            mem::write<glow_mode>(address + 0x02c4, default_mode);
        }
    }

    bool is_visible() {
        if (!valid()) return false;
        float last_visible_time = mem::read<float>(address + offsets::visibility_check);
        float current_time = mem::read<float>(BASE + offsets::globals + 0x28);
        bool visible = last_visible_time > current_time - 0.2f;
        return visible;
    }

    void set_glow_color(vec3 color) {
        vec3 current_color = mem::read<vec3>(address + 0x1d0);
        if (current_color == color) return;
        mem::write<vec3>(address + 0x1d0, color);
    }

    bool valid() {
        if (!address) return false;
        if (get_script_name() < -1000 || get_script_name() > 100000) return false;

        return true;
    }

    float get_max_health() {
        return mem::read<int>(address + offsets::entity::max_health);
    }

    // Gets the position of the entity.
    // If the entity is a character, this will be the position of the feet.
    vec3 get_position() {
        return mem::read<vec3>(address + offsets::entity::abs_origin);
    }

    vec3 get_collision_min() {
        return mem::read<vec3>(address + offsets::entity::collision_min);
    }

    vec3 get_collision_max() {
        return mem::read<vec3>(address + offsets::entity::collision_max);
    }

    // Gets the class name of this entity.
    inline std::string get_class_name() {
        auto client_networkable_vtable = mem::read<uintptr_t>(address + 3 * 8);
        auto get_client_entity = mem::read<uintptr_t>(client_networkable_vtable + 3 * 8);
        auto offset = mem::read<uint32_t>(get_client_entity + 3);
        auto network_name_ptr = mem::read<uintptr_t>(get_client_entity + offset + 7 + 16);

        std::string result;
        for (int i = 0; i < 32; ++i)
        {
            char c = mem::read<char>(network_name_ptr + i);
            result += c;
            if (c == '\0')
                break;
        }
        return result;
    }

    // Gets the type of this entity.
    inline std::string get_type() {
        std::string result;
        uintptr_t buffer = mem::read<uintptr_t>(address + offsets::entity::name);
        if (!buffer) return "";
        for (int i = 0; i < 32; ++i)
        {
            char c = mem::read<char>(buffer + i);
            result.push_back(c);
            if (c == '\0') break;
        }
        return result;
    }

    inline static Entity get_by_id(int id) {
        uintptr_t entity_list = mem::read<uintptr_t>(BASE + offsets::entity_list);
        if (id < 0 || id >= NUM_ENTS || !entity_list)
            return Entity();
        CEntInfo entity_info = mem::read<CEntInfo>(BASE + offsets::entity_list + (id * sizeof(CEntInfo)));
        return Entity(entity_info.pEntity, id);
    }
};

class Weapon : public Entity {
    public:
    Weapon() : Entity() {};
    Weapon(uintptr_t address) : Entity(address, -1) {};

    inline float get_speed() {
        return mem::read<float>(address + offsets::weapon::projectile_speed);
    }

    inline float get_gravity_scale() {
        return mem::read<float>(address + offsets::weapon::projectile_gravity);
    }

    inline float get_gravity() {
        return get_gravity_scale() * 750.0f;
    }

    inline float get_zoom_fov() {
        return mem::read<float>(address + offsets::zoom_fov);
    }

    inline bool ready_to_fire() {
        float current_time = mem::read<float>(BASE + offsets::globals + 0x28);
        float interval = mem::read<float>(BASE + offsets::globals + 0x30);
        float next_ready_time = mem::read<float>(address + 0x1648);
        float next_primary_time = mem::read<float>(address + 0x164c);
        float ready_time = next_ready_time > next_primary_time ? next_ready_time : next_primary_time;
        if (current_time >= ready_time - interval) {
            return true;
        }
        return false;
    }
};

struct kbutton_t {
    int down[2];
    int state;
};

struct Button {
    uintptr_t address;
    kbutton_t button;
    bool state;
    bool force;
    bool press;
    bool release;

    Button(uintptr_t address) : address(address), force(false), press(false), release(false) {
    }

    inline void update() {
        button = mem::read<kbutton_t>(address);
        state = (button.state & 1) != 0;
    }

    inline void post() {
        if (force) {
            int state;
            if (press && !release) {
                state = 5;
            }
            else if (!press && release) {
                state = 4;
            }
            else  {
                state = button.down[0] == 0 && button.down[1] == 0 ? 4 : 5;
            }

            if (state != button.state) {
                mem::write<int>(address + 8, state);
            }
        }
    }
};

inline Button InAttack(BASE + offsets::in_attack);

class Player : public Entity {
    public:
    Player() : Entity() {};
    Player(uintptr_t address, int index) : Entity(address, index) {};

    inline uint16_t get_bone_id(const uint32_t &hitbox) {
        auto model = mem::read<uintptr_t>(address + offsets::studiohdr);
        if (!model) return 0;

        auto studiohdr = mem::read<uintptr_t>(model + 0x8);
        if (!studiohdr) return 0;

        auto hitbox_cache = mem::read<uint16_t>(studiohdr + 0x34);
        if (!hitbox_cache) return 0;

        auto hitbox_array = studiohdr + ((uint16_t)(hitbox_cache & 0xFFFE) << (4 * (hitbox_cache & 1)));
        if (!hitbox_array) return 0;

        auto index_cache = mem::read<uint16_t>(hitbox_array + 4);
        if (!index_cache) return 0;

        auto hitbox_index = ((uint16_t)(index_cache & 0xFFFE) << (4 * (index_cache & 1)));
        if (!hitbox_index) return 0;

        auto bone = mem::read<uint16_t>(hitbox_index + hitbox_array + (hitbox * 0x20));
        return bone;
    }

    // Get the bone position of a specified hitbox.
    // Arguments:
    // hitbox: The hitbox to get the bone position of. Defaults to 0 (head).
    inline vec3 get_bone(int hitbox = 0) {
        uintptr_t bone_array = mem::read<uintptr_t>(address + offsets::entity::bone_array);
        if (!bone_array) return this->get_position();

        int bone_id = get_bone_id(hitbox);
        if (!bone_id) return this->get_position();

        mat3x4 bone = mem::read<mat3x4>(bone_array + (bone_id * sizeof(mat3x4)));
        return bone.translation() + this->get_position();
    }

    // Local player only.
    inline vec3 get_camera_position() {
        return mem::read<vec3>(address + offsets::entity::camera_origin);
    }

    inline vec3 get_angles() {
        return mem::read<vec3>(address + offsets::entity::angles);
    }

    inline vec3 get_recoil() {
        return mem::read<vec3>(address + offsets::entity::aimpunch);
    }

    inline vec3 get_sway() {
        vec3 sway = mem::read<vec3>(address + 0x2588);
        return sway - get_angles();
    }

    inline void set_angles(vec3 angles) {
        if (angles.is_valid())
            return mem::write<vec3>(address + offsets::entity::angles, math::normalize_angles(angles));
    }

    inline vec3 get_velocity() {
        return mem::read<vec3>(address + offsets::entity::velocity);
    }

    inline bool is_zooming() {
        return mem::read<bool>(address + offsets::entity::zooming);
    }

    inline Weapon get_held_weapon() {
        if (!this->valid()) return Weapon();

        uintptr_t weapon = mem::read<uintptr_t>(this->address + offsets::entity::latest_primary_weapons);
        int index = weapon & 0xffff;
        
        if (index != 0xffff) {
            return Weapon(Entity::get_by_id(index).get_address());
        }

        return Weapon();
    }

    void shoot() {
        bool ready_to_fire = this->get_held_weapon().ready_to_fire();
        if (ready_to_fire) {
            InAttack.force = true;
            InAttack.press = true;
            InAttack.post();

        }
    }
};

namespace NetChannel {
    inline size_t get_choked_packets() {
        return mem::read<size_t>(BASE + offsets::netchannel + 0x2028);
    }

    inline void choke(bool state) {
        mem::write<double>(BASE + offsets::netchannel + 0x2108, state ? FLT_MAX : 0);
    }
}

namespace Game {
    inline float get_current_time() {
        return mem::read<float>(BASE + offsets::globals + 0x28);
    }
}

namespace utils {
inline bool get_box_coords(Entity entity, vec4& out) {

    vec3 coords[8];

    vec3 min = entity.get_collision_min();
    vec3 max = entity.get_collision_max();
    vec3 origin = entity.get_position();

    coords[0] = min;
    coords[1] = max;
    coords[2] = vec3(min.x, min.y, max.z);
    coords[3] = vec3(min.x, max.y, min.z);
    coords[4] = vec3(min.x, max.y, max.z);
    coords[5] = vec3(max.x, min.y, min.z);
    coords[6] = vec3(max.x, min.y, max.z);
    coords[7] = vec3(max.x, max.y, min.z);

    vec2 screen_coords[8];

    for (int i = 0; i < 8; i++) {
        if (!math::world_to_screen(coords[i] + origin, screen_coords[i]))
            return false;
    }

    vec4 box = {1920, 1080, 0, 0};

    for (int i = 0; i < 8; i++) {
        if (screen_coords[i].x < box.x)
            box.x = screen_coords[i].x;
        if (screen_coords[i].y < box.y)
            box.y = screen_coords[i].y;
        if (screen_coords[i].x > box.z)
            box.z = screen_coords[i].x;
        if (screen_coords[i].y > box.w)
            box.w = screen_coords[i].y;
    }
    
    out = box;
    return true;
}
}