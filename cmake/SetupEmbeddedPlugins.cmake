# Setup embedded plugins
function (add_embedded_plugins TARGET)
get_property(_embedded_plugins GLOBAL PROPERTY _pr_embedded_plugins)
get_property(_embedded_plugins_src GLOBAL PROPERTY _pr_embedded_plugins_src)
get_property(_embedded_plugins_libs GLOBAL PROPERTY _pr_embedded_plugins_libs)
get_property(_embedded_plugins_includes GLOBAL PROPERTY _pr_embedded_plugins_includes)

if(_embedded_plugins)
  message(STATUS "Embedding plugins: ${_embedded_plugins}")
endif()

# Create header file
set(dst "${CMAKE_BINARY_DIR}/_pr_embedded_plugins.h")
set(_gen_file "extern \"C\" \{ ")
foreach(_e ${_embedded_plugins})
  set(_gen_file "${_gen_file}extern PR::PluginInterface _pr_exports_${_e};")
endforeach()

set(_gen_file "${_gen_file}\}\nstruct\{ const char* Name; PR::PluginInterface* Interface;\} __embedded_plugins[] =\{")
foreach(_e ${_embedded_plugins})
  set(_gen_file "${_gen_file}\{\"${_e}\", &_pr_exports_${_e}\},")
endforeach()
set(_gen_file "${_gen_file}\{nullptr,nullptr\}\};")

file(GENERATE OUTPUT ${dst} CONTENT "${_gen_file}")

list(APPEND _embedded_plugins_src "${dst}")

# Setup necessary definitions per file
foreach(_pl ${_embedded_plugins})
  get_property(_embedded_plugins_${_pl}_src GLOBAL PROPERTY _pr_embedded_plugins_${_pl}_src)
  # Has to be set in the same directory as the target
  set_source_files_properties(${_embedded_plugins_${_pl}_src} PROPERTIES
    COMPILE_DEFINITIONS "PR_EMBED_PLUGIN;_PR_PLUGIN_NAME=${_pl};PR_PLUGIN_VERSION=\"${PR_PLUGIN_VERSION}\"")
endforeach()

target_sources(${TARGET} PRIVATE ${_embedded_plugins_src})
if(_embedded_plugins_libs)
  target_link_libraries(${TARGET} PRIVATE ${_embedded_plugins_libs})
endif()
if(_embedded_plugins_includes)
  target_include_directories(${TARGET} PRIVATE ${_embedded_plugins_includes})
endif()
endfunction()