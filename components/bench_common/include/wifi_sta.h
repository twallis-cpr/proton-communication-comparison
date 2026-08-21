#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Blocks until STA is connected, retry limit is reached, or timeout expires.
 * @param static_ip set to null for DHCP, otherwise use IPV4
 */
esp_err_t wifi_sta_start_and_wait(TickType_t timeout, const char * static_ip);

#ifdef __cplusplus
}
#endif
