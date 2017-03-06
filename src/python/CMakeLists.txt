CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(Src
  camera.cpp
  entity.cpp
  environment.cpp
  geometry.cpp
  light.cpp
  logger.cpp
  mat.cpp
  material.cpp
  module.cpp
  output.cpp
  ray.cpp
  renderentities.cpp
  renderer.cpp
  sampler.cpp
  scene.cpp
  settings.cpp
  shader.cpp
  spectral.cpp
  status.cpp
  tonemapper.cpp
  vec.cpp)
SET(Hdr )

include_directories(../utils)

add_library(pypearray SHARED ${Src} ${Hdr})
target_link_libraries(pypearray pr_lib ${Boost_LIBRARIES})
target_link_libraries(pypearray pr_lib_utils)
set_target_properties(pypearray PROPERTIES VERSION ${PR_Version})
set_target_properties(pypearray PROPERTIES PREFIX "" )
set_target_properties(pypearray PROPERTIES DEBUG_POSTFIX "" )
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