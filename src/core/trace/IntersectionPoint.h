#pragma once

#include "geometry/GeometryPoint.h"
#include "math/Scattering.h"
#include "math/Tangent.h"
#include "ray/Ray.h"

namespace PR {
enum IntersectionPointFlags {
	//IPF_Inside	 = 0x1,
	IPF_IsSurface = 0x2,
	IPF_IsMedium  = 0x4
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
	float NdotV = 0.0f;
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
	uint32 Flags = 0;
	float Depth2 = 0.0f;

	inline void setForPoint(const Vector3f& p)
	{
		P	   = p;
		Depth2 = 0;
		Flags  = 0;
	}

	// Set shading for surfaces terms without transformation
	inline void setForSurface(const PR::Ray& ray, const Vector3f& p, const GeometryPoint& pt)
	{
		P				 = p;
		Ray				 = ray;
		Surface.Geometry = pt;
		Depth2			 = (ray.Origin - P).squaredNorm();
		Flags			 = IPF_IsSurface;

		// TODO: Displacement?
		Surface.P	  = p;
		Surface.NdotV = ray.Direction.dot(pt.N);
		Surface.N	  = pt.N;
		Surface.Nx	  = pt.Nx;
		Surface.Ny	  = pt.Ny;
	}

	// Set shading for medium
	inline void setForMedium(const PR::Ray& ray, const Vector3f& p, uint32 mediumID)
	{
		PR_ASSERT(mediumID != PR_INVALID_ID, "Expected valid medium id");

		P				= p;
		Ray				= ray;
		Depth2			= (ray.Origin - P).squaredNorm();
		Flags			= IPF_IsMedium;
		Medium.MediumID = mediumID;
	}

	inline bool isAtMedium() const { return Flags & IPF_IsMedium; }
	inline bool isAtSurface() const { return Flags & IPF_IsSurface; }

	static inline IntersectionPoint forPoint(const Vector3f& p)
	{
		IntersectionPoint ip;
		ip.setForPoint(p);
		return ip;
	}

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

	inline PR::Ray nextRay(const Vector3f& d, int ray_flags, float minT, float maxT) const
	{
		if (isAtSurface()) {
			const Vector3f oN = d.dot(Surface.N) < 0 ? -Surface.N : Surface.N; // Offset normal used for safe positioning
			return Ray.next(P, d, oN, ray_flags, minT, maxT);
		} else {
			return Ray.next(P, d, ray_flags, minT, maxT);
		}
	}
};
} // namespace PR
