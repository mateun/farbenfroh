# Expose a virtual engine_interface library, so
# game clients can easier access the engine sub-libs and include directories:
add_library(engine_common INTERFACE)
target_include_directories(engine_common INTERFACE
        include
        renderer/include renderer/vulkan/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../v2025/extlibs/glm-0.9.9.8/glm
)
