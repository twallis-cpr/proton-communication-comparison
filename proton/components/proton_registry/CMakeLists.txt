idf_component_register(
    SRCS
        "target_node.c"
        "target_registry.c"
    INCLUDE_DIRS "include"
    REQUIRES
        proton_core
)
