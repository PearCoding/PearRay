CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(Src
  entity.cpp
  logger.cpp
  module.cpp
  pymath.cpp
  ray.cpp
  spectral.cpp)

SET(Hdr
  pymath.h)


add_library(pypearray SHARED ${Src} ${Hdr})
target_link_libraries(pypearray pr_lib pr_lib_utils)
set_target_properties(pypearray PROPERTIES VERSION ${PR_Version})
set_target_properties(pypearray PROPERTIES PREFIX "" )
target_compile_definitions(pypearray PRIVATE "${DEFS}" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")

#file(COPY python/pearray DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(SETUP_PY ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
set(TIMESTAMP ${CMAKE_CURRENT_BINARY_DIR}/build/timestamp)
configure_file(setup.py.in ${SETUP_PY})

#add_custom_command(OUTPUT ${TIMESTAMP}
#                      COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} build
#                      COMMAND ${CMAKE_COMMAND} -E touch ${TIMESTAMP}
#                      DEPENDS pypearray)

#add_custom_target(pypearray_module ALL DEPENDS ${TIMESTAMP})

install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} install --prefix ${CMAKE_INSTALL_PREFIX})")