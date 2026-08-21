set(ZENOH_PICO_DIR "${CMAKE_CURRENT_LIST_DIR}/zenoh-pico")

# ---------------------------------------------------------------------------
# Apply local patches to the zenoh-pico submodule at configure time.
#
# The submodule is pinned to an upstream commit; changes for ESP-IDF
# compatibility live as .patch files in ./patches/ and are applied here.
# Each patch is skipped if it is already applied (reverse-apply check passes),
# so re-running CMake is idempotent.
# ---------------------------------------------------------------------------
find_package(Git QUIET REQUIRED)
file(GLOB _zp_patches "${CMAKE_CURRENT_LIST_DIR}/patches/*.patch")
list(SORT _zp_patches)
foreach(_zp_patch IN LISTS _zp_patches)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" apply --reverse --check "${_zp_patch}"
        WORKING_DIRECTORY "${ZENOH_PICO_DIR}"
        RESULT_VARIABLE _zp_already_applied
        OUTPUT_QUIET ERROR_QUIET
    )
    if(_zp_already_applied EQUAL 0)
        message(STATUS "zenoh-pico: patch already applied: ${_zp_patch}")
    else()
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" apply --check "${_zp_patch}"
            WORKING_DIRECTORY "${ZENOH_PICO_DIR}"
            RESULT_VARIABLE _zp_can_apply
            OUTPUT_QUIET ERROR_QUIET
        )
        if(NOT _zp_can_apply EQUAL 0)
            message(FATAL_ERROR
                "zenoh-pico: cannot apply patch ${_zp_patch} - the submodule "
                "may be at an unexpected revision. Run "
                "'git submodule update --init --recursive' and re-run CMake.")
        endif()
        message(STATUS "zenoh-pico: applying patch ${_zp_patch}")
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" apply "${_zp_patch}"
            WORKING_DIRECTORY "${ZENOH_PICO_DIR}"
            RESULT_VARIABLE _zp_apply_res
        )
        if(NOT _zp_apply_res EQUAL 0)
            message(FATAL_ERROR "zenoh-pico: failed to apply patch ${_zp_patch}")
        endif()
    endif()
endforeach()

# ---------------------------------------------------------------------------
# Generate zenoh-pico/config.h from the upstream template.
#
# zenoh-pico 1.10+ expects a full config.h (with Z_RUNTIME_MAX_TASKS,
# Z_FEATURE_*, ...). Upstream normally produces it via configure_file(); do
# the same here so ESP-IDF users don't have to hand-maintain the file.
# The generated header lands in ${CMAKE_CURRENT_BINARY_DIR}/gen_include/ and
# that path is added to INCLUDE_DIRS below.
# ---------------------------------------------------------------------------

# Buffers / timeouts
set(FRAG_MAX_SIZE                 4096)
set(BATCH_UNICAST_SIZE            2048)
set(BATCH_MULTICAST_SIZE          2048)
set(Z_CONFIG_SOCKET_TIMEOUT       5000)
set(Z_TRANSPORT_LEASE             10000)
set(Z_TRANSPORT_LEASE_EXPIRE_FACTOR 3)
set(Z_RUNTIME_MAX_TASKS           64)
set(Z_RUNTIME_IDLE_READ_TASK_SLEEP 0)
set(Z_TRANSPORT_ACCEPT_TIMEOUT    1000)
set(Z_TRANSPORT_CONNECT_TIMEOUT   10000)

# Features (defaults mirror upstream CMakeLists.txt)
set(Z_FEATURE_CONNECTIVITY            0)
set(Z_FEATURE_MULTI_THREAD            1)
set(Z_FEATURE_PUBLICATION             1)
set(Z_FEATURE_ADVANCED_PUBLICATION    0)
set(Z_FEATURE_SUBSCRIPTION            1)
set(Z_FEATURE_ADVANCED_SUBSCRIPTION   0)
set(Z_FEATURE_QUERY                   1)
set(Z_FEATURE_QUERYABLE               1)
set(Z_FEATURE_LIVELINESS              1)
set(Z_FEATURE_RAWETH_TRANSPORT        0)
set(Z_FEATURE_INTEREST                1)
set(Z_FEATURE_LINK_TCP                1)
set(Z_FEATURE_LINK_BLUETOOTH          0)
set(Z_FEATURE_LINK_WS                 0)
set(Z_FEATURE_LINK_SERIAL             0)
set(Z_FEATURE_LINK_SERIAL_USB         0)
set(Z_FEATURE_LINK_TLS                0)
# NOTE: UDP multicast (and therefore scouting) is disabled: zenoh-pico's
# lwip multicast code assumes IPv4-only LWIP semantics (ip_2_ip4 on an
# ip4_addr_t*), which fails to compile against ESP-IDF's dual-stack LWIP
# build (CONFIG_LWIP_IPV6=y).
set(Z_FEATURE_SCOUTING                0)
set(Z_FEATURE_LINK_UDP_MULTICAST      0)
set(Z_FEATURE_LINK_UDP_UNICAST        1)
set(Z_FEATURE_MULTICAST_TRANSPORT     0)
set(Z_FEATURE_UNICAST_TRANSPORT       1)
set(Z_FEATURE_FRAGMENTATION           1)
set(Z_FEATURE_ENCODING_VALUES         1)
set(Z_FEATURE_TCP_NODELAY             1)
set(Z_FEATURE_LOCAL_SUBSCRIBER        0)
set(Z_FEATURE_LOCAL_QUERYABLE         0)
set(Z_FEATURE_SESSION_CHECK           1)
set(Z_FEATURE_BATCHING                1)
set(Z_FEATURE_BATCH_TX_MUTEX          0)
set(Z_FEATURE_BATCH_PEER_MUTEX        0)
set(Z_FEATURE_MATCHING                1)
set(Z_FEATURE_RX_CACHE                0)
set(Z_FEATURE_UNICAST_PEER            1)
set(Z_FEATURE_AUTO_RECONNECT          1)
set(Z_FEATURE_MULTICAST_DECLARATIONS  0)
set(Z_FEATURE_ADMIN_SPACE             0)
# Z_FEATURE_UNSTABLE_API and ZP_SYSTEM_PLATFORM_HEADER are intentionally left
# undefined so #cmakedefine expands to /* #undef ... */.

set(_zp_config_gen_dir "${CMAKE_CURRENT_BINARY_DIR}/gen_include")
configure_file(
    "${ZENOH_PICO_DIR}/include/zenoh-pico/config.h.in"
    "${_zp_config_gen_dir}/zenoh-pico/config.h"
    @ONLY
)

# ---------------------------------------------------------------------------
# Collect zenoh-pico sources.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE PICO_SRCS_COMMON
    "${ZENOH_PICO_DIR}/src/api/*.c"
    "${ZENOH_PICO_DIR}/src/collections/*.c"
    "${ZENOH_PICO_DIR}/src/link/*.c"
    "${ZENOH_PICO_DIR}/src/net/*.c"
    "${ZENOH_PICO_DIR}/src/protocol/*.c"
    "${ZENOH_PICO_DIR}/src/session/*.c"
    "${ZENOH_PICO_DIR}/src/transport/*.c"
    "${ZENOH_PICO_DIR}/src/utils/*.c"
    "${ZENOH_PICO_DIR}/src/system/common/*.c"
)

# Platform layer: FreeRTOS + LWIP (provided by ESP-IDF).
# system.c is shared with FreeRTOS+TCP and picks the right stack from the
# ZENOH_FREERTOS_LWIP / ZENOH_FREERTOS_PLUS_TCP macros. The LWIP socket
# implementation moved to src/system/socket/lwip.c in zenoh-pico 1.10.
list(APPEND PICO_SRCS_COMMON
    "${ZENOH_PICO_DIR}/src/system/freertos/system.c"
    "${ZENOH_PICO_DIR}/src/system/socket/lwip.c"
)

idf_component_register(
    SRCS ${PICO_SRCS_COMMON}
    INCLUDE_DIRS
        "${_zp_config_gen_dir}"
        "${ZENOH_PICO_DIR}/include"
    REQUIRES freertos lwip esp_driver_uart
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC
    ZENOH_FREERTOS_LWIP
    Z_LOG_LEVEL_DEFAULT=Z_LOG_LEVEL_TRACE
)

# system/platform/freertos/lwip.h uses unprefixed FreeRTOS includes
# (`#include "FreeRTOS.h"`, `semphr.h`, `event_groups.h`). ESP-IDF exposes
# these under a `freertos/` prefix, so add the FreeRTOS-Kernel `freertos/`
# subdirectory to the include path to satisfy the unprefixed lookups.
idf_component_get_property(_zp_freertos_dir freertos COMPONENT_DIR)
target_include_directories(${COMPONENT_LIB} PUBLIC
    "${_zp_freertos_dir}/FreeRTOS-Kernel/include/freertos"
)

# Relax a few warnings that ESP-IDF promotes to errors:
# - unused-but-set-parameter/variable: zenoh-pico's static hashmap templates
#   set an internal `idx` variable in some code paths but do not read it.
# - incompatible-pointer-types: udp_lwip.c uses `unsigned int` for the addrlen
#   argument to recvfrom(), but ESP-IDF's LWIP typedefs socklen_t as
#   `unsigned long int`. The types are ABI-compatible on this target.
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=unused-but-set-parameter
    -Wno-error=unused-but-set-variable
    -Wno-error=incompatible-pointer-types
)
