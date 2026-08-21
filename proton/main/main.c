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
    // Block until the STA netif has an IP; wifi_sta_start_and_wait may return
    // before esp_netif has published the address depending on static/DHCP mode.
    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ip_info = { 0 };
    while (sta_netif == NULL ||
           esp_netif_get_ip_info(sta_netif, &ip_info) != ESP_OK ||
           ip_info.ip.addr == 0) {
        vTaskDelay(pdMS_TO_TICKS(200));
        if (sta_netif == NULL) {
            sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        }
    }
    ESP_LOGI(TAG, "STA up at " IPSTR ", opening UDP socket to %s:%d",
             IP2STR(&ip_info.ip),
             PROTON_NODE_PC_ENDPOINT_0_TRANSPORT_IP,
             PROTON_NODE_PC_ENDPOINT_0_TRANSPORT_PORT);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        ESP_LOGE(TAG, "socket() failed: errno=%d", errno);
        vTaskDelete(NULL);
        return;
    }
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        ESP_LOGE(TAG, "fcntl O_NONBLOCK failed: errno=%d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    const struct sockaddr_in dest = {
        .sin_family = AF_INET,
        .sin_port = htons(PROTON_NODE_PC_ENDPOINT_0_TRANSPORT_PORT),
        // Generator emits the address as a network-byte-order u32 literal.
        .sin_addr.s_addr = PROTON_NODE_PC_ENDPOINT_0_TRANSPORT_IPNL,
    };

    uint8_t send_buf[1024] = { 0 };

    const TickType_t period = pdMS_TO_TICKS(PROTON_TASK_PERIOD_MS);
    TickType_t last_wake = xTaskGetTickCount();

    uint64_t bundle_seq = 0;

    for (;;) {
        int64_t uptime_us = esp_timer_get_time();
        int64_t uptime_s = uptime_us / 1000000LL;
        uint64_t uptime_ms = uptime_us / 1000LL;
        int64_t uptime_ns = (uptime_us % 1000000LL) * 1000LL;

        ImuData_t imu_data;
		if (xQueueReceive(imu_queue, &imu_data, 0) == pdPASS) {
            proton_signal_set_double(&g_proton_registry, PROTON_SIGNAL_ANG_VEL_COVAR_ID, imu_data.ang_vel_covar);
            proton_signal_set_double(&g_proton_registry, PROTON_SIGNAL_ANGULAR_X_ID, imu_data.angular_vel_x);
            proton_signal_set_double(&g_proton_registry, PROTON_SIGNAL_ANGULAR_Y_ID, imu_data.angular_vel_y);
            proton_signal_set_double(&g_proton_registry, PROTON_SIGNAL_ANGULAR_Z_ID, imu_data.angular_vel_z);
            proton_signal_set_double(&g_proton_registry, PROTON_SIGNAL_LINEAR_ACCEL_COVAR_ID, imu_data.linear_accel_covar);
            proton_signal_set_double(&g_proton_registry, PROTON_SIGNAL_LINEAR_X_ID, imu_data.linear_accel_x);
            proton_signal_set_double(&g_proton_registry, PROTON_SIGNAL_LINEAR_Y_ID, imu_data.linear_accel_y);
            proton_signal_set_double(&g_proton_registry, PROTON_SIGNAL_LINEAR_Z_ID, imu_data.linear_accel_z);

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

        if (status == PROTON_OK && out_len > 0) {
            proton_udp4_header_t header;
            proton_udp4_fill_header(&header, dest_peers[0].node_id, 0);
            memcpy(send_buf, &header, sizeof(header));

            ESP_LOGI(TAG, "Sending IMU bundle: %" PRIu64, bundle_seq);
            bundle_seq++;

            ssize_t sent = sendto(sock, send_buf, sizeof(header) + out_len, 0,
                                  (const struct sockaddr *)&dest, sizeof(dest));
            if (sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                ESP_LOGW(TAG, "sendto failed: errno=%d", errno);
            }
        } else if (status != PROTON_OK) {
            ESP_LOGW(TAG, "Proton spin failed: %s", proton_status_to_string(status));
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
