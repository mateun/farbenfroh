
file(GLOB_RECURSE vulkan_src src/*.cpp)



add_library(vulkan SHARED
        ${vulkan_src}
)



target_include_directories(vulkan PUBLIC $ENV{VULKAN_SDK}/Include)
target_link_directories(vulkan PUBLIC $ENV{VULKAN_SDK}/Lib)

target_link_libraries(vulkan vulkan-1 util common_renderer shaderc_combinedd)
