cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DZENOHPICO_PLATFORM_ESP32")
# Select the FreeRTOS+LWIP platform layer in zenoh-pico. ZENOH_ESPIDF must NOT be
# defined here: it is checked before ZENOH_FREERTOS_LWIP in system/common/platform.h
# and would pull in system/platform/espidf.h (whose z_clock_t is an incomplete
# struct timespec because <time.h> is not included).
add_definitions(-DZENOH_FREERTOS_LWIP)

project(esp32_zenoh_bench)
