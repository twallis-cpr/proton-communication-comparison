#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represents typical IMU IC measurement axes
 */
typedef struct ImuData {
    float angular_vel_x;
    float angular_vel_y;
    float angular_vel_z;
    float linear_accel_x;
    float linear_accel_y;
    float linear_accel_z;
} ImuData_t;

/**
 * Represents diff drive odometry
 */
// typedef struct WheelOdom {
//     int32_t l_wheel_pos;
//     int32_t l_wheel_vel;
//     int32_t r_wheel_pos;
//     int32_t r_wheel_vel;
// } WheelOdom_t;

#ifdef __cplusplus
}
#endif
