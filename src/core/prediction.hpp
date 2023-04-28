#pragma once

#include "math.h"
#include <cmath>

// Prediction is broken
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
    float v = ctx.launch_velocity;
    float g = ctx.gravity;
    float x = delta.x;
    float y = delta.y;

    float theta = std::atan((v * v + std::sqrt(v * v * v * v - g * (g * x * x + 2 * y * v * v))) / (g * x));
    if (std::isnan(theta)) {
        return false;
    }
    theta += 90 * TO_RADIANS;
    
    *pitch = theta;

    return true;
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
    // Calculate travel time to current target
    vec2 displacement = {std::sqrt(ctx.target_velocity.x * ctx.target_velocity.x + ctx.target_velocity.y * ctx.target_velocity.y), ctx.target_velocity.z};
    float flight_time = displacement.x / (std::cos(ctx.pitch) * ctx.launch_velocity);

    // Calculate new travel time after extrapolation
    vec3 extrapolated = vec3::extrapolate(ctx.target_position, ctx.target_velocity, flight_time);
    displacement = {std::sqrt(extrapolated.x * extrapolated.x + extrapolated.y * extrapolated.y), extrapolated.z};

    flight_time = displacement.x / (std::cos(ctx.pitch) * ctx.launch_velocity);

    ctx.success = solve_trajectory(ctx.target_position, extrapolated, ctx);
    return ctx.success;
}
} // namespace siege::prediction