set(Src
  camera.cpp
  entity.cpp
  environment.cpp
  geometry.cpp
  infinitelight.cpp
  logger.cpp
  material.cpp
  math.cpp
  module.cpp
  output.cpp
  renderer.cpp
  sampler.cpp
  scene.cpp
  settings.cpp
  shader.cpp
  spectral.cpp
  status.cpp
  tonemapper.cpp
  integrator.cpp
  profile.cpp)
set(Hdr
  pypearray.h)

pybind11_add_module(pypearray SHARED ${Src} ${Hdr})
target_link_libraries(pypearray PRIVATE pr_lib_loader)
set_target_properties(pypearray PROPERTIES DEBUG_POSTFIX "")# Disable suffix for this target
#target_include_directories(pypearray PRIVATE ${OpenEXR_INCLUDE_DIRS})# For lmathVec.h

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