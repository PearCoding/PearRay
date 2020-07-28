PR_ADD_PLUGIN(inf_distant CPP distant.cpp)
PR_ADD_PLUGIN(inf_environment CPP environment.cpp)
PR_ADD_PLUGIN(inf_sky CPP sky.cpp LIBRARIES pr_lib_skysun)
PR_ADD_PLUGIN(inf_sun CPP sun.cpp LIBRARIES pr_lib_skysun)