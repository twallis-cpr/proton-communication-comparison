idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        bench_common
        log
        esp_timer
        zenoh-pico
)

add_definitions(-DZENOHPICO_PLATFORM_ESP32)
