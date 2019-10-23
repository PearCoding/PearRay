#pragma once

#include "geometry/GeometryPoint.h"
#include "math/Reflection.h"
#include "ray/RayPackage.h"

namespace PR {
enum ShaderPointFlags {
	SPF_Inside = 0x1
};

/*
Shading context
- Depending on the function not all fields are reasonable filled.
- View dependent!
*/
struct PR_LIB_INLINE ShadingPoint {
public:
	GeometryPoint Geometry;
	PR::Ray Ray;

	Vector3f P; // Position after displacement
	Vector3f N; // Shading normal - Front facing

	// Normal Tangent Frame
	Vector3f Nx;
	Vector3f Ny;

	// Spectral
	float Radiance;

	// Some other utility variables
	uint32 Flags;
	float Depth2;
	float NdotV;

	uint32 EntityID;
	uint32 PrimID;

	// Set shading terms without transformation
	inline void setByIdentity(const PR::Ray& ray, const GeometryPoint& pt)
	{
		Ray		 = ray;
		Geometry = pt;
		P		 = pt.P;
		Depth2   = (ray.Origin - P).squaredNorm();
		Flags	= 0;

		NdotV = ray.Direction.dot(pt.N);
		if (Reflection::is_inside(NdotV)) {
			N	 = -pt.N;
			Nx	= pt.Nx;
			Ny	= -pt.Ny;
			NdotV = -NdotV;
			Flags |= SPF_Inside;
		} else {
			N  = pt.N;
			Nx = pt.Nx;
			Ny = pt.Ny;
		}
	}
};
} // namespace PR
