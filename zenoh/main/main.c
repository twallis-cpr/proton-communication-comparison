#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <zenoh-pico.h>

#include "data_types.h"
#include "imu_gen_task.h"
#include "wifi_sta.h"

#define ZENOH_TASK_PERIOD_MS 10
#define ZENOH_KEYEXPR        "esp32/imu"

static char *TAG = "esp32_zenoh_bench";

QueueHandle_t imu_queue = NULL;

// Wire-format IMU sample sent as the zenoh publisher payload. Packed so its
// size matches the on-wire layout across compilers.
typedef struct __attribute__((packed)) {
    int64_t stamp_sec;
    int64_t stamp_nsec;
    double  angular_vel_x;
    double  angular_vel_y;
    double  angular_vel_z;
    double  ang_vel_covar;
    double  linear_accel_x;
    double  linear_accel_y;
    double  linear_accel_z;
    double  linear_accel_covar;
} bench_imu_payload_t;

static void wait_for_sta_ip(void)
{
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
    ESP_LOGI(TAG, "STA up at " IPSTR, IP2STR(&ip_info.ip));
}

void zenoh_task(void *args)
{
    (void)args;

    wait_for_sta_ip();

    char locator[64];
    snprintf(locator, sizeof(locator), "tcp/%s:%d",
             CONFIG_BENCH_TARGET_IPV4, CONFIG_BENCH_TARGET_PORT);
    ESP_LOGI(TAG, "Connecting to zenoh router at %s", locator);

    z_owned_config_t config;
    z_config_default(&config);
    zp_config_insert(z_loan_mut(config), Z_CONFIG_MODE_KEY, "client");
    zp_config_insert(z_loan_mut(config), Z_CONFIG_CONNECT_KEY, locator);

    z_owned_session_t session;
	z_result_t ret = z_open(&session, z_move(config), NULL);
    if (ret < 0) {
        ESP_LOGE(TAG, "z_open failed: %d", ret);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Zenoh session open");

    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str_unchecked(&ke, ZENOH_KEYEXPR);

    z_owned_publisher_t pub;
    if (z_declare_publisher(z_loan(session), &pub, z_loan(ke), NULL) < 0) {
        ESP_LOGE(TAG, "z_declare_publisher failed for '%s'", ZENOH_KEYEXPR);
        z_drop(z_move(session));
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Publisher declared on '%s'", ZENOH_KEYEXPR);

    const TickType_t period = pdMS_TO_TICKS(ZENOH_TASK_PERIOD_MS);
    TickType_t last_wake = xTaskGetTickCount();

    bench_imu_payload_t sample = { 0 };
    uint64_t seq = 0;

    for (;;) {
        ImuData_t imu_data;
        if (xQueueReceive(imu_queue, &imu_data, 0) == pdPASS) {
            sample.angular_vel_x      = imu_data.angular_vel_x;
            sample.angular_vel_y      = imu_data.angular_vel_y;
            sample.angular_vel_z      = imu_data.angular_vel_z;
            sample.ang_vel_covar      = imu_data.ang_vel_covar;
            sample.linear_accel_x     = imu_data.linear_accel_x;
            sample.linear_accel_y     = imu_data.linear_accel_y;
            sample.linear_accel_z     = imu_data.linear_accel_z;
            sample.linear_accel_covar = imu_data.linear_accel_covar;
        }

        int64_t uptime_us = esp_timer_get_time();
        sample.stamp_sec  = uptime_us / 1000000LL;
        sample.stamp_nsec = (uptime_us % 1000000LL) * 1000LL;

        z_owned_bytes_t payload;
        if (z_bytes_copy_from_buf(&payload, (const uint8_t *)&sample,
                                  sizeof(sample)) < 0) {
            ESP_LOGW(TAG, "z_bytes_copy_from_buf failed");
        } else {
            ESP_LOGI(TAG, "Publishing %" PRIu64, seq);
            if (z_publisher_put(z_loan(pub), z_move(payload), NULL) < 0) {
                ESP_LOGW(TAG, "z_publisher_put failed");
            }
        }
        seq++;

        vTaskDelayUntil(&last_wake, period);
    }

    z_drop(z_move(pub));
    z_drop(z_move(session));
    vTaskDelete(NULL);
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
