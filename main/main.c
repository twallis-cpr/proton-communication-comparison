#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "rng_task.h"
#include "wifi_sta.h"

static const char *TAG = "app";

void app_main(void)
{
    ESP_LOGI(TAG, "esp32_proton_bench booting");

    esp_err_t err = wifi_sta_start_and_wait(portMAX_DELAY);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "wifi connect failed (%s), continuing without network",
                 esp_err_to_name(err));
    }

    rng_task_start();
}
