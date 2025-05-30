if(NOT ozz_build_fbx)
  return()
endif()

add_library(ozz_animation_fbx
  ${PROJECT_SOURCE_DIR}/include/ozz/animation/offline/fbx/export.h
  ${PROJECT_SOURCE_DIR}/include/ozz/animation/offline/fbx/fbx.h
  fbx.cc
  ${PROJECT_SOURCE_DIR}/include/ozz/animation/offline/fbx/fbx_animation.h
  fbx_animation.cc
  ${PROJECT_SOURCE_DIR}/include/ozz/animation/offline/fbx/fbx_skeleton.h
  fbx_skeleton.cc)
  
target_compile_definitions(ozz_animation_fbx PRIVATE  $<$<BOOL:${BUILD_SHARED_LIBS}>:OZZ_BUILD_ANIMATIONFBX_LIB>)

target_link_libraries(ozz_animation_fbx
  ozz_animation_offline
  fbx::sdk)
set_target_properties(ozz_animation_fbx PROPERTIES FOLDER "ozz/tools")

install(TARGETS ozz_animation_fbx DESTINATION lib)

fuse_target("ozz_animation_fbx")

add_executable(fbx2ozz
  fbx2ozz.h
  fbx2ozz.cc
  fbx2ozz_anim.cc
  fbx2ozz_skel.cc)
target_link_libraries(fbx2ozz
  ozz_animation_tools
  ozz_animation_fbx)
target_copy_shared_libraries(fbx2ozz)
set_target_properties(fbx2ozz
  PROPERTIES FOLDER "ozz/tools")

install(TARGETS fbx2ozz DESTINATION bin/tools)

# Gerenate binary data to a specific folder (${ozz_media_directory}/bin)
set(build_configurations
  # Library data
  "/fbx/pab/skeleton.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":true}},\"animations\":[]}\;output:pab_skeleton.ozz"
  "/fbx/pab/atlas.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"filename\":\"pab_atlas_raw.ozz\",\"raw\":true,\"optimize\":false}]}\;output:pab_atlas_raw.ozz\;depend:pab_skeleton.ozz"
  "/fbx/pab/atlas.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"filename\":\"pab_atlas.ozz\"}]}\;output:pab_atlas.ozz\;depend:pab_skeleton.ozz"
  "/fbx/pab/locomotions.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"filename\":\"pab_*.ozz\"}]}\;output:pab_walk.ozz\;output:pab_jog.ozz\;output:pab_run.ozz\;depend:pab_skeleton.ozz"
  "/fbx/pab/locomotions.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"filename\":\"pab_*_raw.ozz\",\"raw\":true,\"optimize\":false}]}\;output:pab_walk_raw.ozz\;output:pab_jog_raw.ozz\;output:pab_run_raw.ozz\;depend:pab_skeleton.ozz"
  "/fbx/pab/crossarms.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"filename\":\"pab_crossarms.ozz\"}]}\;output:pab_crossarms.ozz\;depend:pab_skeleton.ozz"
  "/fbx/pab/crackhead.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"filename\":\"pab_crackhead.ozz\"},{\"filename\":\"pab_crackhead_additive.ozz\",\"additive\":true}]}\;output:pab_crackhead_additive.ozz\;output:pab_crackhead.ozz\;depend:pab_skeleton.ozz"
  "/fbx/pab/hand.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"clip\":\"curl\",\"filename\":\"pab_curl_additive.ozz\",\"additive\":true,\"additive_reference\":\"skeleton\"},{\"clip\":\"splay\",\"filename\":\"pab_splay_additive.ozz\",\"additive\":true,\"additive_reference\":\"skeleton\"}]}\;output:pab_curl_additive.ozz\;output:pab_splay_additive.ozz\;depend:pab_skeleton.ozz"
  
  # Library data with root motion
  "/fbx/pab/atlas.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"filename\":\"pab_atlas_no_motion.ozz\",\"tracks\":{\"motion\":{\"enable\":true,\"rotation\":{\"loop\":true},\"filename\":\"pab_atlas_motion_track.ozz\"}}}]}\;output:pab_atlas_no_motion.ozz\;output:pab_atlas_motion_track.ozz\;depend:pab_skeleton.ozz"
  "/fbx/pab/locomotions.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"filename\":\"pab_*_no_motion.ozz\",\"tracks\":{\"motion\":{\"enable\":true,\"rotation\":{\"components\":\"\"},\"filename\":\"pab_*_motion_track.ozz\"}}}]}\;output:pab_walk_no_motion.ozz\;output:pab_jog_no_motion.ozz\;output:pab_run_no_motion.ozz\;output:pab_walk_motion_track.ozz\;output:pab_jog_motion_track.ozz\;output:pab_run_motion_track.ozz\;depend:pab_skeleton.ozz"
  
  # Robot user channels
  "/fbx/robot.fbx\;\;config_file:${PROJECT_SOURCE_DIR}/samples/user_channel/config.json\;output:robot_skeleton.ozz\;output:robot_animation.ozz\;output:robot_track_grasp.ozz"

  # Baked animation
  "/fbx/baked.fbx\;\;config_file:${PROJECT_SOURCE_DIR}/samples/baked/config.json\;output:baked_skeleton.ozz\;output:baked_animation.ozz"
  
  # Ruby
  "/fbx/sketchfab/ruby.fbx\;{\"skeleton\":{\"filename\":\"ruby_skeleton.ozz\",\"import\":{\"enable\":true}},\"animations\":[{\"filename\":\"ruby_animation.ozz\"}]}\;output:ruby_skeleton.ozz\;output:ruby_animation.ozz"

  # Versioning
  "/fbx/pab/skeleton.fbx\;{\"skeleton\":{\"filename\":\"versioning/raw_skeleton_v1_le.ozz\",\"import\":{\"enable\":true,\"raw\":true}},\"animations\":[]}\;output:versioning/raw_skeleton_v1_le.ozz\;option:--endian=little"
  "/fbx/pab/skeleton.fbx\;{\"skeleton\":{\"filename\":\"versioning/raw_skeleton_v1_be.ozz\",\"import\":{\"enable\":true,\"raw\":true}},\"animations\":[]}\;output:versioning/raw_skeleton_v1_be.ozz\;option:--endian=big"
  "/fbx/pab/skeleton.fbx\;{\"skeleton\":{\"filename\":\"versioning/skeleton_v2_le.ozz\",\"import\":{\"enable\":true}},\"animations\":[]}\;output:versioning/skeleton_v2_le.ozz\;option:--endian=little"
  "/fbx/pab/skeleton.fbx\;{\"skeleton\":{\"filename\":\"versioning/skeleton_v2_be.ozz\",\"import\":{\"enable\":true}},\"animations\":[]}\;output:versioning/skeleton_v2_be.ozz\;option:--endian=big"
  "/fbx/pab/run.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"filename\":\"versioning/raw_animation_v3_le.ozz\",\"raw\":true}]}\;output:versioning/raw_animation_v3_le.ozz\;option:--endian=little\;depend:pab_skeleton.ozz"
  "/fbx/pab/run.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"filename\":\"versioning/raw_animation_v3_be.ozz\",\"raw\":true}]}\;output:versioning/raw_animation_v3_be.ozz\;option:--endian=big\;depend:pab_skeleton.ozz"
  "/fbx/pab/run.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"filename\":\"versioning/animation_v7_le.ozz\"}]}\;output:versioning/animation_v7_le.ozz\;option:--endian=little\;depend:pab_skeleton.ozz"
  "/fbx/pab/run.fbx\;{\"skeleton\":{\"filename\":\"pab_skeleton.ozz\",\"import\":{\"enable\":false}},\"animations\":[{\"filename\":\"versioning/animation_v7_be.ozz\"}]}\;output:versioning/animation_v7_be.ozz\;option:--endian=big\;depend:pab_skeleton.ozz"

  # Collada
  "/collada/astro_max.dae\;{\"skeleton\":{\"filename\":\"astro_max_skeleton.ozz\",\"import\":{\"enable\":true}},\"animations\":[{\"filename\":\"astro_max_animation.ozz\"}]}\;output:astro_max_animation.ozz\;output:astro_max_skeleton.ozz"
  "/collada/astro_maya.dae\;{\"skeleton\":{\"filename\":\"astro_maya_skeleton.ozz\",\"import\":{\"enable\":true}},\"animations\":[{\"filename\":\"astro_maya_animation.ozz\"}]}\;output:astro_maya_animation.ozz\;output:astro_maya_skeleton.ozz"
  "/collada/seymour.dae\;{\"skeleton\":{\"filename\":\"seymour_skeleton.ozz\",\"import\":{\"enable\":true}},\"animations\":[{\"filename\":\"seymour_animation.ozz\"}]}\;output:seymour_animation.ozz\;output:seymour_skeleton.ozz"
)

# Generate a command for each line of the configuration
foreach(line ${build_configurations})

  # First 3 elements are mandatory
  list(GET line 0 src)
  list(GET line 1 config)

  # Optional elements
  list(LENGTH line line_length)

  # Loops over all remaining arguments to find outputs.
  set(outputs "")
  set(options "")
  set(depends "")
  set(config_file "")
    
  # Loops over all elements to find parameters.
  foreach(element ${line})
    if (${element} MATCHES "output:(.*)")
      string(SUBSTRING ${element} 7 -1 output)
      list(APPEND outputs "${ozz_media_directory}/bin/${output}")
    endif()
    if (${element} MATCHES "depend:(.*)")
      string(SUBSTRING ${element} 7 -1 depend)
      list(APPEND depends "${ozz_media_directory}/bin/${depend}")
    endif()
    if (${element} MATCHES "option:(.*)")
      string(SUBSTRING ${element} 7 -1 option)
      list(APPEND options ${option})
    endif()
    if (${element} MATCHES "config_file:(.*)")
      string(SUBSTRING ${element} 12 -1 config_file)
    endif()
  endforeach()

  list(APPEND all_outputs ${outputs})
  # output is required
  if (NOT outputs)
    message("Missing output file(s) list.")
  endif()

  # Dump config file
  if(NOT config_file)
    string(MD5 hash "${line}")
    set(config_file "${ozz_temp_directory}/config_${hash}.json")
    file(WRITE "${config_file}" "${config}")
  else()
    list(APPEND depends "${config_file}")
  endif()

  # Create build command
  add_custom_command(
    DEPENDS "${ozz_media_directory}${src}"
            ${depends}
            $<$<AND:$<BOOL:${ozz_build_data}>,$<BOOL:${ozz_build_fbx}>>:fbx2ozz>
    OUTPUT  ${outputs}
    COMMAND fbx2ozz
            "--file=${ozz_media_directory}${src}"
            "--config_file=${config_file}"
            ${options}
    WORKING_DIRECTORY "${ozz_media_directory}/bin/"
    VERBATIM)
endforeach()

# Creates a target to build data
add_custom_target(BUILD_DATA ALL
  DEPENDS ${all_outputs}
  VERBATIM)
