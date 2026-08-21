#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "data_types.h"
#include "imu_gen_task.h"
#include "wifi_sta.h"


static char *TAG = "esp32_zenoh_bench";

QueueHandle_t imu_queue = NULL;

void zenoh_task(void *args)
{
    (void)args;
}

void app_main(void)
{
    ESP_LOGI(TAG, "%s booting", TAG);

    esp_err_t err = wifi_sta_start_and_wait(portMAX_DELAY, NULL);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "wifi connect failed (%s), continuing without network",
                 esp_err_to_name(err));
    }

	imu_queue = xQueueCreate(4, sizeof(ImuData_t));

	if (imu_queue != NULL) {
		xTaskCreate(zenoh_task,
				"zenoh_task",
				CONFIG_ZENOH_TASK_STACK,
				(void*)imu_queue,
				CONFIG_ZENOH_TASK_PRIO,
				NULL);

		imu_gen_task_start((void*)imu_queue);
	} else {
		ESP_LOGE(TAG, "IMU queue is null, time to explode!");
		ESP_ERROR_CHECK(-1);
	}
}
