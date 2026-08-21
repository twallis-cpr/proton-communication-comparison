#include "imu_gen_task.h"
#include "data_types.h"

#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define IMU_GEN_TASK_STACK_SIZE 3072
#define IMU_GEN_TASK_PERIOD_MS  10

// static const char *TAG = "imu_gen_task";

static void imu_gen_task(void *arg)
{
    QueueHandle_t imu_queue = (QueueHandle_t)arg;
    const TickType_t period = pdMS_TO_TICKS(IMU_GEN_TASK_PERIOD_MS);
    TickType_t last_wake = xTaskGetTickCount();

    const float sine_axis_offset = M_PI / 6.0f;
    const float sine_step = M_PI / 40.0f;

    float sine_input = 0.0f;
    float axes[6] = {0.0f};
    ImuData_t imu;

    for (;;) {
        for (uint8_t i = 0; i < 6; i++) {
            axes[i] = sin(sine_input + sine_axis_offset * i);
        }

        sine_input += sine_step;

        imu.ang_vel_covar = axes[0];
        imu.angular_vel_x = axes[0];
        imu.angular_vel_y = axes[1];
        imu.angular_vel_z = axes[2];

        imu.linear_accel_covar = axes[3];
        imu.linear_accel_x = axes[3];
        imu.linear_accel_y = axes[4];
        imu.linear_accel_z = axes[5];

        xQueueSend(imu_queue, (void*)&imu, 0);

        vTaskDelayUntil(&last_wake, period);
    }
}

void imu_gen_task_start(void *arg)
{
    xTaskCreate(imu_gen_task, "imu_gen_task", IMU_GEN_TASK_STACK_SIZE, arg,
                tskIDLE_PRIORITY + 2, NULL);
}
