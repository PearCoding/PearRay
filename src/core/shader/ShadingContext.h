#pragma once

#include "material/MaterialContext.h"

namespace PR {

// Class representing the context a shading socket as access to
class PR_LIB_CORE ShadingContext {
public:
	Vector2f UV		   = Vector2f(0, 0);
	Vector2f dUV	   = Vector2f(0, 0);
	uint32 Face		   = 0; // Useful for PTex
	uint32 ThreadIndex = 0;
	bool IsInside	   = false;
	SpectralBlob WavelengthNM;

	inline static ShadingContext fromIP(uint32 thread_index, const IntersectionPoint& pt)
	{
		return ShadingContext{ pt.Surface.Geometry.UV, Vector2f::Zero(), pt.Surface.Geometry.PrimitiveID, thread_index,
							   static_cast<bool>(pt.Flags & IPF_Inside), pt.Ray.WavelengthNM };
	}

	inline static ShadingContext fromMC(uint32 thread_index, const MaterialSampleContext& ctx)
	{
		return ShadingContext{ ctx.UV, Vector2f::Zero(), ctx.PrimitiveID, thread_index,
							   ctx.IsInside, ctx.WavelengthNM };
	}
};
} // namespace PR
