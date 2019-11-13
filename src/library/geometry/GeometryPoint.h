#pragma once

#include "PR_Config.h"

namespace PR {
/*
Geometry context (SOA)
- View independent!
*/
struct PR_LIB GeometryPoint {
	// Point of sample
	Vector3f P;
	Vector3f dPdT; // Velocity of P

	Vector3f N;

	// Normal Tangent Frame
	Vector3f Nx;
	Vector3f Ny;

	// 2D surface parameters
	Vector3f UVW;
	Vector3f dUVW; // Pixel footprint

	uint32 MaterialID;
	uint32 EmissionID;
	uint32 DisplaceID;// TODO
};
} // namespace PR
