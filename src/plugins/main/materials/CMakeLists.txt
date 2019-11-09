CMAKE_MINIMUM_REQUIRED(VERSION 3.3)

PR_ADD_PLUGIN(mat_lambert CPP lambert.cpp)
PR_ADD_PLUGIN(mat_orennayar CPP orennayar.cpp)
PR_ADD_PLUGIN(mat_glass CPP glass.cpp)
PR_ADD_PLUGIN(mat_mirror CPP mirror.cpp)
PR_ADD_PLUGIN(mat_ward CPP ward.cpp)
PR_ADD_PLUGIN(mat_microfacet CPP microfacet.cpp)
PR_ADD_PLUGIN(mat_principled CPP principled.cpp)