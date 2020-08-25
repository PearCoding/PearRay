#pragma once

#include "geometry/GeometryPoint.h"
#include "math/Scattering.h"
#include "math/Tangent.h"
#include "ray/Ray.h"

namespace PR {
enum IntersectionPointFlags {
	IPF_Inside	 = 0x1,
	IPF_IsMedium = 0x2
};

class PR_LIB_CORE SurfaceIntersectionPoint {
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

class PR_LIB_CORE MediumIntersectionPoint {
public:
	uint32 MediumID;
};

/*
Shading context
- Depending on the function not all fields are reasonable filled.
- View dependent!
*/
class PR_LIB_CORE IntersectionPoint {
public:
	Vector3f P;
	//union {
	SurfaceIntersectionPoint Surface;
	MediumIntersectionPoint Medium;
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

		// TODO: Displacement?
		Surface.P	  = p;
		Surface.NdotV = ray.Direction.dot(pt.N);
		Surface.N	  = pt.N;
		Surface.Nx	  = pt.Nx;
		Surface.Ny	  = pt.Ny;
		if (Scattering::is_inside_global(Surface.NdotV)) {
			Tangent::invert_frame(Surface.N, Surface.Nx, Surface.Ny);
			Surface.NdotV = -Surface.NdotV;
			Flags |= IPF_Inside;
		}
	}

	// Set shading for medium
	inline void setForMedium(const PR::Ray& ray, const Vector3f& p, uint32 mediumID)
	{
		P				= p;
		Ray				= ray;
		Depth2			= (ray.Origin - P).squaredNorm();
		Flags			= IPF_IsMedium;
		Medium.MediumID = mediumID;
	}

	inline bool isAtMedium() const { return Flags & IPF_IsMedium; }
	inline bool isAtSurface() const { return !isAtMedium(); }

	static inline IntersectionPoint forSurface(const PR::Ray& ray, const Vector3f& p, const GeometryPoint& pt)
	{
		IntersectionPoint ip;
		ip.setForSurface(ray, p, pt);
		return ip;
	}

	static inline IntersectionPoint forMedium(const PR::Ray& ray, const Vector3f& p, uint32 mediumID)
	{
		IntersectionPoint ip;
		ip.setForMedium(ray, p, mediumID);
		return ip;
	}
};
} // namespace PR
