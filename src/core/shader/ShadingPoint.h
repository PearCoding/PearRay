#pragma once

#include "geometry/GeometryPoint.h"
#include "math/Reflection.h"
#include "math/Tangent.h"
#include "ray/Ray.h"

namespace PR {
enum ShaderPointFlags {
	SPF_Inside	 = 0x1,
	SPF_IsMedium = 0x2
};

class PR_LIB_CORE ShadingSurfacePoint {
public:
	GeometryPoint Geometry;
	Vector3f P; // Position after displacement
	Vector3f N; // Shading normal - Front facing

	// Normal Tangent Frame
	Vector3f Nx;
	Vector3f Ny;

	// Cached member
	float NdotV;
};

class PR_LIB_CORE ShadingMediumPoint {
public:
	uint32 MediumID;
};

/*
Shading context
- Depending on the function not all fields are reasonable filled.
- View dependent!
*/
class PR_LIB_CORE ShadingPoint {
public:
	Vector3f P;
	//union {
	ShadingSurfacePoint Surface;
	ShadingMediumPoint Medium;
	//};
	PR::Ray Ray;

	// Some other utility variables
	uint32 Flags;
	float Depth2;

	// Set shading for surfaces terms without transformation
	inline void setForSurface(const PR::Ray& ray, const Vector3f& p, const GeometryPoint& pt)
	{
		P				 = p;
		Ray				 = ray;
		Surface.Geometry = pt;
		Depth2			 = (ray.Origin - P).squaredNorm();
		Flags			 = 0;

		Surface.NdotV = ray.Direction.dot(pt.N);
		Surface.N	  = pt.N;
		Surface.Nx	  = pt.Nx;
		Surface.Ny	  = pt.Ny;
		if (Reflection::is_inside(Surface.NdotV)) {
			Tangent::invert_frame(Surface.N, Surface.Nx, Surface.Ny);
			Surface.NdotV = -Surface.NdotV;
			Flags |= SPF_Inside;
		}
	}

	// Set shading for medium
	inline void setForMedium(const PR::Ray& ray, const Vector3f& p, uint32 mediumID)
	{
		P				= p;
		Ray				= ray;
		Depth2			= (ray.Origin - P).squaredNorm();
		Flags			= SPF_IsMedium;
		Medium.MediumID = mediumID;
	}

	inline bool isAtMedium() const { return Flags & SPF_IsMedium; }
	inline bool isAtSurface() const { return !isAtMedium(); }
};
} // namespace PR
