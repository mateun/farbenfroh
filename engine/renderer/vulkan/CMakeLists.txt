
file(GLOB_RECURSE vulkan_src src/*.cpp)



add_library(vulkan STATIC
        ${vulkan_src}
)



target_include_directories(vulkan PUBLIC $ENV{VULKAN_SDK}/Include include)
target_link_directories(vulkan PUBLIC $ENV{VULKAN_SDK}/Lib)

target_link_libraries(vulkan engine_common vulkan-1 util common_renderer shaderc_combinedd)
