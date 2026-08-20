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

#ifdef __cplusplus
}
#endif
