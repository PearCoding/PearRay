add_custom_target(pr_plugins ALL)

function(PR_ADD_PLUGIN NAME)
  cmake_parse_arguments(_args "" "" "CPP;LIBRARIES" ${ARGN})
  set(TARGET pr_pl_${NAME})

  add_library(${TARGET} MODULE ${_args_CPP})
  target_link_libraries(${TARGET} PRIVATE pr_lib pr_lib_utils)
  if(_args_LIBRARIES)
    target_link_libraries(${TARGET} PRIVATE ${_args_LIBRARIES})
  endif()
  set_target_properties(${TARGET} PROPERTIES CXX_VISIBILITY_PRESET hidden)
  target_compile_definitions(${TARGET} PRIVATE "${DEFS};PR_PLUGIN_BUILD" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")
  add_lto(${TARGET})
  strip_binary(${TARGET})

  add_dependencies(pr_plugins ${TARGET})
endfunction()

add_definitions(-DPR_PLUGIN_VERSION="1.0")

IF(PR_WITH_MAIN_PLUGINS)
  add_subdirectory(main)
ENDIF(PR_WITH_MAIN_PLUGINS)

IF(PR_WITH_EXTRA_PLUGINS)
  add_subdirectory(extra)
ENDIF(PR_WITH_EXTRA_PLUGINS)