add_custom_target(pr_plugins ALL)

set(_target_prefix pr_pl_)
set_property(GLOBAL PROPERTY _pr_embedded_plugins )
set_property(GLOBAL PROPERTY _pr_embedded_plugins_src )
set_property(GLOBAL PROPERTY _pr_embedded_plugins_libs )
function(PR_ADD_PLUGIN NAME)
  cmake_parse_arguments(_args "" "" "CPP;LIBRARIES" ${ARGN})
  set(TARGET ${_target_prefix}${NAME})

  if(PR_EMBED_PLUGINS)
    set(_abs_files )
    foreach(_f ${_args_CPP})
      get_filename_component(_file "${_f}" ABSOLUTE)
      list(APPEND _abs_files "${_file}")
    endforeach()

    get_property(_embedded_plugins_src GLOBAL PROPERTY _pr_embedded_plugins_src)
    list(APPEND _embedded_plugins_src ${_abs_files})
    set_property(GLOBAL PROPERTY _pr_embedded_plugins_src "${_embedded_plugins_src}" )

    list(APPEND _embedded_plugins_${NAME}_src ${_abs_files})
    set_property(GLOBAL PROPERTY _pr_embedded_plugins_${NAME}_src "${_embedded_plugins_${NAME}_src}" )

    get_property(_embedded_plugins_libs GLOBAL PROPERTY _pr_embedded_plugins_libs)
    list(APPEND _embedded_plugins_libs "${_args_LIBRARIES}")
    set_property(GLOBAL PROPERTY _pr_embedded_plugins_libs "${_embedded_plugins_libs}" )

    get_property(_embedded_plugins GLOBAL PROPERTY _pr_embedded_plugins)
    list(APPEND _embedded_plugins "${NAME}")
    set_property(GLOBAL PROPERTY _pr_embedded_plugins "${_embedded_plugins}" )
  else()# Shared library
    add_library(${TARGET} MODULE ${_args_CPP})
    target_link_libraries(${TARGET} PRIVATE pr_lib_utils)
    set_target_properties(${TARGET} PROPERTIES CXX_VISIBILITY_PRESET hidden)

    if(_args_LIBRARIES)
      target_link_libraries(${TARGET} PRIVATE ${_args_LIBRARIES})
    endif()

    target_compile_definitions(${TARGET} PRIVATE
      "PR_PLUGIN_BUILD"
      "_PR_PLUGIN_NAME=${NAME}"
      "PR_PLUGIN_VERSION=\"${PR_PLUGIN_VERSION}\""
      "$<$<CONFIG:Debug>:PR_DEBUG>")

    add_lto(${TARGET})
    strip_binary(${TARGET})

    add_dependencies(pr_plugins ${TARGET})
  endif()
endfunction()

if(PR_WITH_MAIN_PLUGINS)
  add_subdirectory(main)
endif(PR_WITH_MAIN_PLUGINS)

if(PR_WITH_EXTRA_PLUGINS)
  add_subdirectory(extra)
endif(PR_WITH_EXTRA_PLUGINS)
