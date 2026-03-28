set(CMAKE_AUTOMOC ON)

# ─── Expressway Core ──────────────────────────────────────────────
add_library(axiom_expressway_core STATIC
    core/vulkan_init.cpp
)

target_link_libraries(axiom_expressway_core PUBLIC
    Vulkan::Vulkan
)

target_include_directories(axiom_expressway_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ─── Render Graph (Placeholder) ──────────────────────────────────
add_library(axiom_expressway_rendergraph STATIC
    core/vulkan_init.cpp # Placeholder source
)

target_link_libraries(axiom_expressway_rendergraph PUBLIC
    axiom_expressway_core
)

# ─── Expressway Scene & Bridge ────────────────────────────────────
add_library(axiom_expressway_scene STATIC
    scene/axiom_expressway_node.cpp
    include/expressway/axiom_expressway_node.h
)

target_link_libraries(axiom_expressway_scene PUBLIC
    axiom_expressway_rendergraph
    Qt6::Quick
)

# Full Expressway Alias
add_library(axiom_expressway INTERFACE)
target_link_libraries(axiom_expressway INTERFACE axiom_expressway_scene)
