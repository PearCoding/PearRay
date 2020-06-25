#pragma once

#include "material/MaterialContext.h"

namespace PR {

// Class representing the context a shading socket as access to
class PR_LIB_CORE ShadingContext {
public:
	Vector2f UV	 = Vector2f(0, 0);
	Vector2f dUV = Vector2f(0, 0);
	uint32 Face	 = 0; // Useful for PTex
	SpectralBlob WavelengthNM;
	bool IsInside;

	inline static ShadingContext fromIP(const IntersectionPoint& pt)
	{
		return ShadingContext{ pt.Surface.Geometry.UV, Vector2f::Zero(), pt.Surface.Geometry.PrimitiveID,
							   pt.Ray.WavelengthNM, static_cast<bool>(pt.Flags & IPF_Inside) };
	}

	inline static ShadingContext fromMC(const MaterialSampleContext& ctx)
	{
		return ShadingContext{ ctx.UV, Vector2f::Zero(), ctx.PrimitiveID,
							   ctx.WavelengthNM, ctx.IsInside };
	}
};
} // namespace PR
