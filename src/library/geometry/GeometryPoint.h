#pragma once

#include "PR_Config.h"

namespace PR {
/*
Geometry context (SOA)
- View independent!
*/
struct PR_LIB_INLINE GeometryPoint {
public:
	// Point of sample
	Vector3f P;
	Vector3f Pd;   // Position after displacement
	Vector3f dPdT; // Velocity of P

	Vector3f Ng; // Geometric normal.
	Vector3f Nd; // Normal after displacement

	// Normal Tangent Frame
	Vector3f Nx;
	Vector3f Ny;

	// 2D surface parameters
	Vector3f UVW;
	Vector3f dUVW; // Pixel footprint

	uint32 MaterialID;
};
} // namespace PR
