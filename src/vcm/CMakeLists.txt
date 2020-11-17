# Header only library
add_library(pr_lib_vcm INTERFACE)
target_sources(pr_lib_vcm INTERFACE
    Defaults.h
    Kernel.h
    Options.h
    PathVertex.h
    PathVertexMap.h
    PathVertexMap.h
    RussianRoulette.h
    Tracer.h
    TracerIterationContext.h
    TracerThreadContext.h
    TraversalContext.h
    Utils.h
    Walker.h
    Wavelength.h)
target_link_libraries(pr_lib_vcm INTERFACE pr_lib_loader)
# Calling convention is vcm/
target_include_directories(pr_lib_vcm INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
