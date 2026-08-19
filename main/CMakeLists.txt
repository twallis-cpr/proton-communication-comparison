idf_component_register(
    SRCS
        "main.c"
        "wifi_sta.c"
        "rng_task.c"
    INCLUDE_DIRS "."
    REQUIRES
        nvs_flash
        esp_wifi
        esp_event
        esp_netif
        esp_hw_support
        log
)
