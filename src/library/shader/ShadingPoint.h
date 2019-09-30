#pragma once

#include "math/SIMD.h"

namespace PR {
enum ShaderClosureFlags {
	SCF_Inside = 0x1
};

/*
Shading context (SOA)
- Depending on the function not all fields are reasonable filled.
- View dependent!
*/
struct PR_LIB_INLINE ShadingPoint {
public:
	// Point of sample
	float P[3];
	float Pd[3];   // Position after displacement
	float dPdT[3]; // Velocity of P

	float Ng[3]; // Geometric normal.
	float Ns[3]; // Shading normal - Front facing
	float Nd[3]; // Normal after displacement

	// Normal Tangent Frame
	float Nx[3];
	float Ny[3];

	// 2D surface parameters
	float UVW[3];
	float dU; // Pixel footprint (U)
	float dV; // Pixel footprint (V)
	float dW; // Pixel footprint (W)

	// Spectral
	uint32 WavelengthIndex;
	float Radiance;

	// Viewing direction (incident)
	float V[3];

	// Some other utility variables
	float Depth2; // Squared!
	float NgdotV;
	float NdotV;
	uint32 Flags;

	uint32 EntityID;
	uint32 PrimID;
	uint32 MaterialID;
};
} // namespace PR
