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

#include "proton/registry.h"
#include "proton/node_manager.h"
#include "proton/transport/udp4.h"
#include "target_connections.h"
#include "target_registry_ids.h"
#include "target_registry_sizes.h"

#define PROTON_TASK_PERIOD_MS 10

static char *TAG = "esp32_proton_bench";

QueueHandle_t imu_queue = NULL;

extern proton_registry_t g_proton_registry;
extern proton_node_t g_target_node;

void proton_task(void *arg)
{
    uint8_t send_buf[1024] = { 0 };

    const TickType_t period = pdMS_TO_TICKS(PROTON_TASK_PERIOD_MS);
    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        int64_t uptime_us = esp_timer_get_time();
        int64_t uptime_s = uptime_us / 1000000LL;
        uint64_t uptime_ms = uptime_us / 1000LL;
        int64_t uptime_ns = (uptime_us % 1000000LL) * 1000LL;

        ImuData_t imu_data;
		if (xQueueReceive(imu_queue, &imu_data, 0) == pdPASS) {
            proton_signal_set_float(&g_proton_registry, PROTON_SIGNAL_ANGULAR_X_ID, imu_data.angular_vel_x);
            proton_signal_set_float(&g_proton_registry, PROTON_SIGNAL_ANGULAR_Y_ID, imu_data.angular_vel_y);
            proton_signal_set_float(&g_proton_registry, PROTON_SIGNAL_ANGULAR_Z_ID, imu_data.angular_vel_z);
            proton_signal_set_float(&g_proton_registry, PROTON_SIGNAL_LINEAR_X_ID, imu_data.linear_accel_x);
            proton_signal_set_float(&g_proton_registry, PROTON_SIGNAL_LINEAR_Y_ID, imu_data.linear_accel_y);
            proton_signal_set_float(&g_proton_registry, PROTON_SIGNAL_LINEAR_Z_ID, imu_data.linear_accel_z);

            proton_signal_set_int64(&g_proton_registry, PROTON_SIGNAL_TS_SEC_ID, uptime_s);
            proton_signal_set_int64(&g_proton_registry, PROTON_SIGNAL_TS_NSEC_ID, uptime_ns);
        }

        size_t out_len = 0;
        size_t num_selected_peers = 0;
        proton_endpoint_t dest_peers[1];

        proton_status_e status = proton_node_update(
            &g_target_node,
            uptime_ms,
            &send_buf[sizeof(proton_udp4_header_t)],
            sizeof(send_buf),
            &out_len,
            dest_peers,
            1,
            &num_selected_peers
        );

        if (status == PROTON_OK) {
            proton_udp4_header_t header;
            proton_udp4_fill_header(&header, dest_peers[0].node_id, 0);
            memcpy(send_buf, &header, sizeof(header));
        }

        vTaskDelayUntil(&last_wake, period);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "esp32_proton_bench booting");

    esp_err_t err = wifi_sta_start_and_wait(portMAX_DELAY, PROTON_NODE_MCU_ENDPOINT_0_TRANSPORT_IP);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "wifi connect failed (%s), continuing without network",
                 esp_err_to_name(err));
    }

    g_target_node.registry = &g_proton_registry;

	imu_queue = xQueueCreate(4, sizeof(ImuData_t));

	if (imu_queue != NULL) {
		xTaskCreate(proton_task,
				"proton_task",
				CONFIG_PROTON_TASK_STACK,
				(void*)imu_queue,
				CONFIG_PROTON_TASK_PRIO,
				NULL);

		imu_gen_task_start((void*)imu_queue);
	} else {
		ESP_LOGE(TAG, "IMU queue is null, time to explode!");
		ESP_ERROR_CHECK(-1);
	}
}
