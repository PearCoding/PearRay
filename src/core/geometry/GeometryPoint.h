#pragma once

#include "PR_Config.h"

namespace PR {
/*
Geometry context (SOA)
- View independent!
*/
struct PR_LIB_CORE GeometryPoint {
	Vector3f N;
	// Normal Tangent Frame
	Vector3f Nx;
	Vector3f Ny;

	// 2D surface parameters
	Vector2f UV;
	Vector2f dUV; // Pixel footprint

	uint32 EntityID; // Will be set automatically
	uint32 PrimitiveID;
	uint32 MaterialID;
	uint32 EmissionID;
	uint32 DisplaceID;// TODO
};
} // namespace PR
