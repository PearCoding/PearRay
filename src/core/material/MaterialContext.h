#pragma once

#include "math/ShadingVector.h"
#include "math/Spherical.h"
#include "trace/IntersectionPoint.h"

namespace PR {
class MaterialEvalContext;

// Class representing the material shading environment
// The normal is oriented as (0,0,1)
class PR_LIB_CORE MaterialSampleContext {
public:
	Vector3f P;		 // Global space
	ShadingVector V; // Outgoing (NOT INCIDENT) view vector in shading space
	Vector2f UV;
	uint32 PrimitiveID; // Useful for PTex
	SpectralBlob WavelengthNM;
	uint32 RayFlags = 0;

	inline float NdotV() const { return V(2); }
	inline float XdotV() const { return V(0); }
	inline float YdotV() const { return V(1); }

	inline static MaterialSampleContext fromIP(const IntersectionPoint& sp)
	{
		PR_ASSERT(sp.isAtSurface(), "Expected IntersectionPoint to be a surface point");

		MaterialSampleContext ctx;
		ctx.P			 = sp.Surface.P;
		ctx.UV			 = sp.Surface.Geometry.UV;
		ctx.PrimitiveID	 = sp.Surface.Geometry.PrimitiveID;
		ctx.WavelengthNM = sp.Ray.WavelengthNM;
		ctx.RayFlags	 = sp.Ray.Flags;
		ctx.setVFromGlobal(sp, -sp.Ray.Direction);

		return ctx;
	}

	inline void setVFromGlobal(const IntersectionPoint& sp, const Vector3f& gV)
	{
		V = Tangent::toTangentSpace(sp.Surface.N, sp.Surface.Nx, sp.Surface.Ny, gV);
	}

	inline MaterialEvalContext expand(const Vector3f& L) const;
};

class PR_LIB_CORE MaterialEvalContext : public MaterialSampleContext {
public:
	ShadingVector L; // Outgoing light vector in shading space

	inline float NdotL() const { return L(2); }
	inline float XdotL() const { return L(0); }
	inline float YdotL() const { return L(1); }

	inline Vector2f computeViewAngles() const { return Spherical::from_direction(V); }
	inline Vector2f computeLightAngles() const { return Spherical::from_direction(L); }

	inline static MaterialEvalContext fromIP(const IntersectionPoint& sp, const Vector3f& gL)
	{
		return fromIP(sp, -sp.Ray.Direction, gL);
	}

	inline static MaterialEvalContext fromIP(const IntersectionPoint& sp, const Vector3f& gV, const Vector3f& gL)
	{
		PR_ASSERT(sp.isAtSurface(), "Expected IntersectionPoint to be a surface point");

		MaterialEvalContext ctx;
		ctx.P			 = sp.Surface.P;
		ctx.UV			 = sp.Surface.Geometry.UV;
		ctx.PrimitiveID	 = sp.Surface.Geometry.PrimitiveID;
		ctx.WavelengthNM = sp.Ray.WavelengthNM;
		ctx.RayFlags	 = sp.Ray.Flags;
		ctx.setVFromGlobal(sp, gV);
		ctx.setLFromGlobal(sp, gL);

		return ctx;
	}

	inline void setLFromGlobal(const IntersectionPoint& sp, const Vector3f& gL)
	{
		L = Tangent::toTangentSpace(sp.Surface.N, sp.Surface.Nx, sp.Surface.Ny, gL);
	}

	inline void swapVL()
	{
		std::swap(V, L);
	}
};

inline MaterialEvalContext MaterialSampleContext::expand(const Vector3f& L) const
{
	MaterialEvalContext ctx;
	*reinterpret_cast<MaterialSampleContext*>(&ctx) = *this;
	ctx.L											= L;
	return ctx;
}

} // namespace PR
