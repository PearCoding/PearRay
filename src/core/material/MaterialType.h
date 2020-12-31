#pragma once

#include "Enum.h"

namespace PR {
/* A material having a diffuse path should never have a specular path and vice versa! */
enum class MaterialScatteringType : uint32 {
	DiffuseReflection = 0,
	SpecularReflection,
	DiffuseTransmission,
	SpecularTransmission
};

/// Type of distribution the scattering is done with
/// TODO: Needs a better name!
enum class MaterialScatter : uint8 {
	Null			  = 0x1,
	DeltaDistribution = 0x2,
	SpectralVarying	  = 0x4,  // TODO
	SpatialVarying	  = 0x8,  // TODO
	TimeVarying		  = 0x10, // TODO
};
PR_MAKE_FLAGS(MaterialScatter, MaterialScatterFlags)

} // namespace PR