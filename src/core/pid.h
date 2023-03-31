#pragma once

class PID {
    private:
    float kp, ki, kd;
    float integral, derivative, last_error;
    float min, max;

    public:
    PID(float kp, float ki, float kd, float min, float max) : kp(kp), ki(ki), kd(kd), min(min), max(max) {
        integral = 0;
        derivative = 0;
        last_error = 0;
    }

    float update(float error, float delta_time) {
        integral += error * delta_time;
        derivative = (error - last_error) / delta_time;
        last_error = error;

        float output = kp * error + ki * integral + kd * derivative;

        if (output > max) output = max;
        if (output < min) output = min;

        return output;
    }
};