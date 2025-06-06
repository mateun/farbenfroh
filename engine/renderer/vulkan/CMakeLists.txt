
file(GLOB_RECURSE vulkan_src src/*.cpp)



add_library(vulkan STATIC
        ${vulkan_src}
)

if(CMAKE_BUILD_TYPE STREQUAL "PerformanceTest")
    message(STATUS "VULKAN validatiaon layers are turned OFF for performance test builds.")
    target_compile_definitions(vulkan PUBLIC NO_VALIDATION_LAYERS)
    message(STATUS "VULKAN queries are turned ON for performance test builds.")
    target_compile_definitions(vulkan PUBLIC PERFORMANCE_TESTS)
endif()


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "VULKAN validatiaon layers are turned off for Release build.")
    target_compile_definitions(vulkan PUBLIC NO_VALIDATION_LAYERS)
endif()


target_include_directories(vulkan PUBLIC $ENV{VULKAN_SDK}/Include include)
target_link_directories(vulkan PUBLIC $ENV{VULKAN_SDK}/Lib)

target_link_libraries(vulkan engine_common vulkan-1 util common_renderer)
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "PerformanceTest")
    target_link_libraries(vulkan shaderc_combined)
    message(STATUS "VULKAN linking to shaderc release library")
else()
    message(STATUS "VULKAN linking to shaderc debug library")
    target_link_libraries(vulkan shaderc_combinedd)
endif()
