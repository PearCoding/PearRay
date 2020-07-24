if(SeExpr2_FOUND)
    PR_ADD_PLUGIN(nod_expr CPP ExprNode.cpp LIBRARIES SeExpr2::SeExpr2)
elseif(PR_EXTRA_SEEXPR)
    message(WARNING "Skipping SeExpr2 based expression node.")
endif()

PR_ADD_PLUGIN(nod_skysun CPP SkySunNode.cpp 
    skymodel/ArHosekSkyModel.h skymodel/ArHosekSkyModel.cpp
    skymodel/ArHosekSkyModelData_Spectral.h skymodel/ArHosekSkyModelData_CIEXYZ.h skymodel/ArHosekSkyModelData_RGB.h)