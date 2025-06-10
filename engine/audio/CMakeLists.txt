file(GLOB_RECURSE audio_src ${CMAKE_CURRENT_SOURCE_DIR}/../../v2025/src/engine/sound/*.cpp)


add_library(audio STATIC
        ${audio_src}
)

target_include_directories(audio
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/src/engine/sound
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
