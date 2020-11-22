# Header only library
set(SRC 
    vcm/Defaults.h
    vcm/Kernel.h
    vcm/Options.h
    vcm/PathVertex.h
    vcm/PathVertexMap.h
    vcm/PathVertexMap.h
    vcm/RussianRoulette.h
    vcm/Tracer.h
    vcm/TracerIterationContext.h
    vcm/TracerThreadContext.h
    vcm/TraversalContext.h
    vcm/Utils.h
    vcm/Walker.h
    vcm/Wavelength.h )

add_library(pr_lib_vcm INTERFACE)
#target_sources(pr_lib_vcm INTERFACE ${SRC})
target_link_libraries(pr_lib_vcm INTERFACE pr_lib_loader)
# Calling convention is vcm/
target_include_directories(pr_lib_vcm INTERFACE 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:${INCLUDE_PREFIX}/>)

set_target_properties(pr_lib_vcm PROPERTIES EXPORT_NAME vcm)
install(TARGETS pr_lib_vcm COMPONENT development EXPORT PearRay-targets)
installHeaderFiles(${SRC})