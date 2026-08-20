idf_component_register(
    SRCS
        "wifi_sta.c"
        "imu_gen_task.c"
    INCLUDE_DIRS "include"
    REQUIRES
        nvs_flash
        esp_wifi
        esp_event
        esp_netif
        esp_hw_support
        log
)
