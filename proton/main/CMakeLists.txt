idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        bench_common
        proton_core
        proton_registry
        log
        esp_timer
)
