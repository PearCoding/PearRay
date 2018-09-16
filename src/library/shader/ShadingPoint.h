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
struct PR_SIMD_ALIGN PR_LIB_INLINE ShadingPoint {
public:
	// Point of sample
	vfloat P[3];
	vfloat Pd[3];   // Position after displacement
	vfloat dPdT[3]; // Velocity of P

	vfloat Ng[3]; // Geometric normal.
	vfloat Ns[3]; // Shading normal - Front facing
	vfloat Nd[3]; // Normal after displacement

	// Normal Tangent Frame
	vfloat Nx[3];
	vfloat Ny[3];

	// 2D surface parameters
	vfloat UVW[3];
	vfloat dU; // Pixel footprint (U)
	vfloat dV; // Pixel footprint (V)
	vfloat dW; // Pixel footprint (W)

	// Spectral
	uint32 WavelengthIndex;// Wavelength is always same for each ray in the package!
	vfloat Radiance;

	// Viewing direction (incident)
	vfloat V[3];

	// Some other utility variables
	vfloat Depth2; // Squared!
	vfloat NgdotV;
	vfloat NdotV;
	vuint32 Flags;

	vuint32 EntityID;
	vuint32 PrimID;
	vuint32 MaterialID;
};
} // namespace PR
