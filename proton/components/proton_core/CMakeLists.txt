# ESP-IDF component wrapper around Clearpath's proton_core (C library).
#
# Upstream is a plain CMake project; this file replaces upstream's
# core/CMakeLists.txt for the ESP-IDF build. Mirrors the source list and
# public include dirs of the proton::proton_core target at tag 2.0.0-beta6,
# using the pre-committed nanopb-generated sources (no protoc at build time).
#
# The static registry files (target_registry.c, target_node.c, *.h) produced
# by generator_scripts/generator.py are per-application and belong in the
# consumer component, not here.

set(PROTON_DIR "${CMAKE_CURRENT_LIST_DIR}/proton")
set(PROTON_CORE_DIR "${PROTON_DIR}/core")
set(NANOPB_DIR "${PROTON_DIR}/external/nanopb")

idf_component_register(
    SRCS
        "${PROTON_CORE_DIR}/src/common.c"
        "${PROTON_CORE_DIR}/src/encode_decode.c"
        "${PROTON_CORE_DIR}/src/node_manager.c"
        "${PROTON_CORE_DIR}/src/proton_field_callbacks.c"
        "${PROTON_CORE_DIR}/src/registry.c"
        "${PROTON_CORE_DIR}/src/transport/serial.c"
        "${PROTON_CORE_DIR}/src/transport/udp4.c"
        "${PROTON_CORE_DIR}/src/generated/signal.pb.c"
        "${PROTON_CORE_DIR}/src/generated/bundle.pb.c"
        "${PROTON_CORE_DIR}/src/generated/proton.pb.c"
        "${NANOPB_DIR}/pb_common.c"
        "${NANOPB_DIR}/pb_decode.c"
        "${NANOPB_DIR}/pb_encode.c"
    INCLUDE_DIRS
        "${PROTON_CORE_DIR}/include"
        "${NANOPB_DIR}"
)
