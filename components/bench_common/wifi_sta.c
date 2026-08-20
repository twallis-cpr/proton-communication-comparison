#include "wifi_sta.h"

#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#if CONFIG_BENCH_WIFI_AUTH_OPEN
#define BENCH_WIFI_AUTH_MODE WIFI_AUTH_OPEN
#elif CONFIG_BENCH_WIFI_AUTH_WEP
#define BENCH_WIFI_AUTH_MODE WIFI_AUTH_WEP
#elif CONFIG_BENCH_WIFI_AUTH_WPA_PSK
#define BENCH_WIFI_AUTH_MODE WIFI_AUTH_WPA_PSK
#elif CONFIG_BENCH_WIFI_AUTH_WPA2_PSK
#define BENCH_WIFI_AUTH_MODE WIFI_AUTH_WPA2_PSK
#elif CONFIG_BENCH_WIFI_AUTH_WPA_WPA2_PSK
#define BENCH_WIFI_AUTH_MODE WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_BENCH_WIFI_AUTH_WPA3_PSK
#define BENCH_WIFI_AUTH_MODE WIFI_AUTH_WPA3_PSK
#elif CONFIG_BENCH_WIFI_AUTH_WPA2_WPA3_PSK
#define BENCH_WIFI_AUTH_MODE WIFI_AUTH_WPA2_WPA3_PSK
#else
#define BENCH_WIFI_AUTH_MODE WIFI_AUTH_WPA2_PSK
#endif

static const char *TAG = "wifi_sta";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < CONFIG_BENCH_WIFI_MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP (%d/%d)",
                     s_retry_num, CONFIG_BENCH_WIFI_MAX_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGW(TAG, "connect to the AP failed");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_sta_start_and_wait(TickType_t timeout)
{
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES || nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_err);

    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        return ESP_ERR_NO_MEM;
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &event_handler, NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &event_handler, NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = BENCH_WIFI_AUTH_MODE,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    strncpy((char *) wifi_config.sta.ssid, CONFIG_BENCH_WIFI_SSID,
            sizeof(wifi_config.sta.ssid));
    strncpy((char *) wifi_config.sta.password, CONFIG_BENCH_WIFI_PASSWORD,
            sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished, waiting for connection");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE, timeout);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s", CONFIG_BENCH_WIFI_SSID);
        return ESP_OK;
    }
    if (bits & WIFI_FAIL_BIT) {
        ESP_LOGW(TAG, "failed to connect to SSID:%s", CONFIG_BENCH_WIFI_SSID);
        return ESP_FAIL;
    }
    ESP_LOGW(TAG, "wifi connect wait timed out");
    return ESP_ERR_TIMEOUT;
}
