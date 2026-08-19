#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

// Blocks until STA is connected, retry limit is reached, or timeout expires.
esp_err_t wifi_sta_start_and_wait(TickType_t timeout);

#ifdef __cplusplus
}
#endif
