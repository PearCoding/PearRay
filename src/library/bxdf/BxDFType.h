#pragma once

#include "PR_Config.h"

namespace PR {
enum BxDFType : uint8 {
	BT_None			= 0x0,
	BT_Diffuse		= 0x1,
	BT_Specular		= 0x2,
	BT_Reflection	= 0x4,
	BT_Transmission = 0x8,

	BT_DiffuseReflection	= BT_Diffuse | BT_Reflection,
	BT_SpecularReflection	= BT_Specular | BT_Reflection,
	BT_DiffuseTransmission	= BT_Diffuse | BT_Transmission,
	BT_SpecularTransmission = BT_Specular | BT_Transmission,

	BT_All = BT_Diffuse | BT_Specular | BT_Reflection | BT_Transmission
};
} // namespace PR