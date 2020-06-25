#pragma once

#include "trace/IntersectionPoint.h"

namespace PR {
class MaterialEvalContext;

// Class representing the material shading environment
// The normal is oriented as (0,0,1)
class PR_LIB_CORE MaterialSampleContext {
public:
	Vector3f P; // Global space
	Vector3f V; // Outgoing (NOT INCIDENT) view vector in shading space
	Vector2f UV;
	uint32 PrimitiveID; // Useful for PTex
	SpectralBlob WavelengthNM;
	bool IsInside;

	inline float NdotV() const { return V(2); }
	inline float XdotV() const { return V(0); }
	inline float YdotV() const { return V(1); }

	inline static MaterialSampleContext fromIP(const IntersectionPoint& sp)
	{
		PR_ASSERT(sp.isAtSurface(), "Expected IntersectionPoint to be a surface point");

		MaterialSampleContext ctx;
		ctx.P			 = sp.Surface.P;
		ctx.V			 = Tangent::toTangentSpace(sp.Surface.N, sp.Surface.Nx, sp.Surface.Ny, -sp.Ray.Direction);
		ctx.UV			 = sp.Surface.Geometry.UV;
		ctx.PrimitiveID	 = sp.Surface.Geometry.PrimitiveID;
		ctx.WavelengthNM = sp.Ray.WavelengthNM;
		ctx.IsInside	 = sp.Flags & IPF_Inside;

		return ctx;
	}

	inline MaterialEvalContext expand(const Vector3f& L) const;
};

class PR_LIB_CORE MaterialEvalContext : public MaterialSampleContext {
public:
	Vector3f L; // Outgoing light vector in shading space
	Vector3f H; // Shading space

	inline void calcH() { H = (V + L).normalized(); }

	inline float NdotL() const { return L(2); }
	inline float XdotL() const { return L(0); }
	inline float YdotL() const { return L(1); }

	inline float NdotH() const { return H(2); }
	inline float XdotH() const { return H(0); }
	inline float YdotH() const { return H(1); }

	//inline float HdotV() const { return 0; /* TODO*/ }
	//inline float HdotL() const { return HdotV(); }

	inline static MaterialEvalContext fromIP(const IntersectionPoint& sp, const Vector3f& gL)
	{
		PR_ASSERT(sp.isAtSurface(), "Expected IntersectionPoint to be a surface point");

		MaterialEvalContext ctx;
		ctx.P			 = sp.Surface.P;
		ctx.UV			 = sp.Surface.Geometry.UV;
		ctx.PrimitiveID	 = sp.Surface.Geometry.PrimitiveID;
		ctx.WavelengthNM = sp.Ray.WavelengthNM;
		ctx.V			 = Tangent::toTangentSpace(sp.Surface.N, sp.Surface.Nx, sp.Surface.Ny, -sp.Ray.Direction);
		ctx.L			 = Tangent::toTangentSpace(sp.Surface.N, sp.Surface.Nx, sp.Surface.Ny, gL);
		ctx.IsInside	 = sp.Flags & IPF_Inside;
		ctx.calcH();

		return ctx;
	}
};

inline MaterialEvalContext MaterialSampleContext::expand(const Vector3f& L) const
{
	MaterialEvalContext ctx;
	*reinterpret_cast<MaterialSampleContext*>(&ctx) = *this;
	ctx.L											= L;
	ctx.calcH();
	return ctx;
}

} // namespace PR
