#pragma once
#include <cstddef>
#include <cstdint>

#define BASE 0x140000000

namespace offsets {
    inline constexpr ptrdiff_t in_attack = 0x07871c50;
    inline constexpr ptrdiff_t local_player_handle = 0x15572fc;

    inline constexpr ptrdiff_t viewrenderer = 0x786f390;
    inline constexpr ptrdiff_t viewmatrix = 0x11a210;
    
    inline constexpr ptrdiff_t studiohdr = 0x10f0;

    inline constexpr ptrdiff_t entity_list = 0x1d0fe58; //cl_entitylist

    inline constexpr ptrdiff_t netchannel = 0x1639800;
    inline constexpr ptrdiff_t commands = 0x20A9960;
    inline constexpr ptrdiff_t current_command = 0x16524BC;
    inline constexpr ptrdiff_t globals = 0x16394a0;

    inline constexpr ptrdiff_t visibility_check = 0x1A78;

    namespace entity {
        constexpr ptrdiff_t health = 0x43c;             //m_iHealth
        constexpr ptrdiff_t shield = 0x170;
        constexpr ptrdiff_t max_shield = 0x174;
        constexpr ptrdiff_t max_health = 0x0578;        //m_iMaxHealth
        constexpr ptrdiff_t bone_array = 0x0e98 + 0x48; //m_nForceBone + 0x48
        constexpr ptrdiff_t collision_min = 0x04c0+0x10;//m_Collision + 0x10
        constexpr ptrdiff_t collision_max = 0x04c0+0x1c;//m_Collision + 0x1c
        constexpr ptrdiff_t abs_origin = 0x14c;         //m_vecAbsOrigin
        constexpr ptrdiff_t velocity = 0x0488;          //m_vecAbsVelocity
        constexpr ptrdiff_t name = 0x0580;              //m_iName
        constexpr ptrdiff_t latest_primary_weapons = 0x1a1c;
        constexpr ptrdiff_t camera_origin = 0x1f48;
        constexpr ptrdiff_t ammo_pool_capacity = 0x25ac;
        constexpr ptrdiff_t angles = ammo_pool_capacity - 0x14;
        constexpr ptrdiff_t sway_angles = ammo_pool_capacity - 0x14 - 0x10;
        constexpr ptrdiff_t zooming = 0x1c51;
        constexpr ptrdiff_t script_index = 0x1648;
        constexpr ptrdiff_t team_id = 0x044c;
        constexpr ptrdiff_t aimpunch = 0x24b0;
    }

    namespace weapon {
        constexpr ptrdiff_t projectile_speed = 0x1f48;   //m_flProjectileSpeed
        constexpr ptrdiff_t projectile_gravity = 0x1f50; //m_flProjectileScale
    }
}