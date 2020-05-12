#pragma once

#include "PR_Config.h"

namespace PR {
/* A material having a diffuse path should never have a specular path and vice versa! */
enum MaterialScatteringType : uint32 {
	MST_DiffuseReflection = 0,
	MST_SpecularReflection,
	MST_DiffuseTransmission,
	MST_SpecularTransmission
};
} // namespace PR