#pragma once

#include "PR_Config.h"
#include "ray/RayPackage.h"
#include "geometry/GeometryPoint.h"

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

	Vector3f Pd; // Position after displacement
	Vector3f Ns; // Shading normal - Front facing

	// Normal Tangent Frame
	Vector3f Nx;
	Vector3f Ny;

	// Spectral
	float Radiance;

	// Some other utility variables
	float Depth2; // Squared!
	float NgdotV;
	float NdotV;
	uint32 Flags;

	uint32 EntityID;
	uint32 PrimID;
};
} // namespace PR
