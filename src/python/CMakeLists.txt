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
  status.cpp
  tonemapper.cpp
  integrator.cpp
  profile.cpp)
set(Hdr
  pypearray.h)

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/api")

pybind11_add_module(pypearray SHARED ${Src} ${Hdr})
target_link_libraries(pypearray PRIVATE pr_lib_loader)
set_target_properties(pypearray PROPERTIES 
                      DEBUG_POSTFIX "" # Disable suffix for this target
                      )
strip_binary(pypearray) # Strip before other post builds
add_custom_command(TARGET pypearray PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}"
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/pearray "${OUTPUT_DIR}/pearray")
add_custom_command(TARGET pypearray POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pypearray> "${OUTPUT_DIR}/pearray")

# TODO: What about non-embedded plugins?

set(SETUP_PY ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
set(TIMESTAMP ${CMAKE_CURRENT_BINARY_DIR}/build/timestamp)
configure_file(setup.py.in ${SETUP_PY})

install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} install --prefix ${CMAKE_INSTALL_PREFIX})")