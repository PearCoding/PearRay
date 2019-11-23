if(OpenSubdiv_FOUND)
    PR_ADD_PLUGIN(ent_subdiv CPP subdiv.cpp LIBRARIES OpenSubdiv::CPU)
endif()