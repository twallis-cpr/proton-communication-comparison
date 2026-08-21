cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DZENOHPICO_PLATFORM_ESP32")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DZENOH_ESPIDF")
add_definitions(-DZENOH_FREERTOS_LWIP)

project(esp32_zenoh_bench)
