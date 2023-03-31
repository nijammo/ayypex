#pragma once

#include "math.h"
#include <cmath>

namespace prediction {
struct prediction_ctx {
        float launch_velocity, gravity;
        vec3 target_velocity;
        vec3 target_position, launch_position;
        vec3 angles;
        float time_of_flight, pitch, distance;
        bool success;
};

inline bool calculate_pitch(prediction_ctx ctx, vec2 delta, float *pitch) {
    float v0 = ctx.launch_velocity;
    float g = ctx.gravity;
    float root = v0 * v0 * v0 * v0 - g * (g * delta.x * delta.x + 2.0f * delta.y * v0 * v0);
    if (root > 0.0f) {
        *pitch = std::atan((v0 * v0 - std::sqrt(root)) / (g * delta.x));
        return true;
    }
    return false;
}

inline bool solve_trajectory(vec3 currentPosition, vec3 extrapolatedPosition, prediction_ctx &ctx) {
    vec3 delta = extrapolatedPosition - currentPosition;
    vec2 delta2d = {std::sqrt(delta.x * delta.x + delta.y * delta.y), delta.z};

    float pitch;
    if (!calculate_pitch(ctx, delta2d, &pitch)) {
        return false;
    }

    ctx.time_of_flight = delta2d.x / (std::cos(pitch) * ctx.launch_velocity);

    ctx.angles.y = std::atan2(delta.y, delta.x) * TO_DEGREES;
    ctx.angles.x = -pitch * TO_DEGREES;
    ctx.angles.z = 0;

    return true;
}

inline bool predict_trajectory(prediction_ctx &ctx) {
    float MAX_TIME = 1.0f, TIME_STEP = (1.0f / 256.0f);
    for (float currentTime = 0.0f; currentTime <= MAX_TIME; currentTime += TIME_STEP) {
        float travelTime = ctx.distance / ctx.launch_velocity;
        vec3 extrapolatedPosition = vec3::extrapolate(ctx.target_position, ctx.target_velocity, travelTime);

        if (!solve_trajectory(ctx.launch_position, extrapolatedPosition, ctx)) {
            continue;
        }
        if (ctx.time_of_flight < currentTime) {
            ctx.success = true;
            return true;
        }
    }
    ctx.success = false;
    return false;
}
} // namespace siege::prediction