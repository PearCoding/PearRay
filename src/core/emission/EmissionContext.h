#pragma once

#include "math/ShadingVector.h"
#include "math/Spherical.h"
#include "trace/IntersectionPoint.h"

namespace PR {
class EmissionEvalContext;

// Class representing the emission shading environment
// The normal is oriented as (0,0,1)
// Contary to the material shading environment no viewing vector is given.
class PR_LIB_CORE EmissionSampleContext {
public:
	Vector3f P; // Global space
	Vector2f UV;
	uint32 PrimitiveID	  = PR_INVALID_ID; // Useful for PTex
	PR::RayFlags RayFlags = 0;

	inline static EmissionSampleContext fromIP(const IntersectionPoint& sp)
	{
		PR_ASSERT(sp.isAtSurface(), "Expected IntersectionPoint to be a surface point");

		EmissionSampleContext ctx;
		ctx.P			 = sp.Surface.P;
		ctx.UV			 = sp.Surface.Geometry.UV;
		ctx.PrimitiveID	 = sp.Surface.Geometry.PrimitiveID;
		ctx.RayFlags	 = sp.Ray.Flags;

		return ctx;
	}

	inline EmissionEvalContext expand(const Vector3f& L, const SpectralBlob& wavelengthNM) const;
};

class PR_LIB_CORE EmissionEvalContext : public EmissionSampleContext {
public:
	ShadingVector L;		   // Outgoing light vector in shading space
	SpectralBlob WavelengthNM; // Light vector wavelength, same as incoming for most materials

	inline float NdotL() const { return L(2); }
	inline float XdotL() const { return L(0); }
	inline float YdotL() const { return L(1); }

	inline Vector2f computeLightAngles() const { return Spherical::from_direction(L); }

	inline static EmissionEvalContext fromIP(const IntersectionPoint& sp, const Vector3f& gL)
	{
		PR_ASSERT(sp.isAtSurface(), "Expected IntersectionPoint to be a surface point");

		EmissionEvalContext ctx;
		ctx.P						= sp.Surface.P;
		ctx.UV						= sp.Surface.Geometry.UV;
		ctx.PrimitiveID				= sp.Surface.Geometry.PrimitiveID;
		ctx.WavelengthNM			= sp.Ray.WavelengthNM;
		ctx.RayFlags				= sp.Ray.Flags;
		ctx.setLFromGlobal(sp, gL);

		return ctx;
	}

	inline void setLFromGlobal(const IntersectionPoint& sp, const Vector3f& gL)
	{
		L = Tangent::toTangentSpace(sp.Surface.N, sp.Surface.Nx, sp.Surface.Ny, gL);
	}
};

inline EmissionEvalContext EmissionSampleContext::expand(const Vector3f& L, const SpectralBlob& wavelengthNM) const
{
	EmissionEvalContext ctx;
	*reinterpret_cast<EmissionSampleContext*>(&ctx) = *this;
	ctx.L											= L;
	ctx.WavelengthNM								= wavelengthNM;
	return ctx;
}

} // namespace PR
