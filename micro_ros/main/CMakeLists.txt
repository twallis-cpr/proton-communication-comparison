idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        bench_common
        log
        micro_ros_espidf_component
)
