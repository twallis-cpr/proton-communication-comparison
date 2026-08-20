#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "data_types.h"
#include "imu_gen_task.h"
#include "wifi_sta.h"

static char *TAG = "esp32_proton_bench";

#define BENCH_STR_(x) #x
#define BENCH_STR(x)  BENCH_STR_(x)

QueueHandle_t imu_queue = NULL;

void app_main(void)
{
    ESP_LOGI(TAG, "esp32_proton_bench booting");

    esp_err_t err = wifi_sta_start_and_wait(portMAX_DELAY);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "wifi connect failed (%s), continuing without network",
                 esp_err_to_name(err));
    }

	imu_queue = xQueueCreate(4, sizeof(ImuData_t));

	if (imu_queue != NULL) {
		//pin micro-ros task in APP_CPU to make PRO_CPU to deal with wifi:
		// xTaskCreate(micro_ros_task,
		// 		"uros_task",
		// 		CONFIG_MICRO_ROS_APP_STACK,
		// 		(void*)imu_queue,
		// 		CONFIG_MICRO_ROS_APP_TASK_PRIO,
		// 		NULL);

		imu_gen_task_start((void*)imu_queue);
	} else {
		ESP_LOGE(TAG, "IMU queue is null, time to explode!");
		ESP_ERROR_CHECK(-1);
	}
}
