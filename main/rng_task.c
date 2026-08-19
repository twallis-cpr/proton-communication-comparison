#include "rng_task.h"

#include <inttypes.h>

#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "rng";

#define RNG_TASK_STACK_SIZE 3072
#define RNG_TASK_PERIOD_MS  500

static void rng_task(void *arg)
{
    (void) arg;
    const TickType_t period = pdMS_TO_TICKS(RNG_TASK_PERIOD_MS);
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        uint32_t val = esp_random();
        ESP_LOGI(TAG, "rng=%08" PRIx32, val);
        vTaskDelayUntil(&last_wake, period);
    }
}

void rng_task_start(void)
{
    xTaskCreate(rng_task, "rng_task", RNG_TASK_STACK_SIZE, NULL,
                tskIDLE_PRIORITY + 2, NULL);
}
