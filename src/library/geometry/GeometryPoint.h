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
	float P[3];
	float Pd[3];   // Position after displacement
	float dPdT[3]; // Velocity of P

	float Ng[3]; // Geometric normal.
	float Nd[3]; // Normal after displacement

	// Normal Tangent Frame
	float Nx[3];
	float Ny[3];

	// 2D surface parameters
	float UVW[3];
	float dUVW[3]; // Pixel footprint

	uint32 MaterialID;

	// Utility functions
	inline void setPosition(const Eigen::Vector3f& p)
	{
		for (int i = 0; i < 3; ++i) {
			P[i]	= p(i);
			Pd[i]   = p(i);
			dPdT[i] = 0;
		}
	}

	inline void setTangentFrame(const Eigen::Vector3f& n,
								const Eigen::Vector3f& t,
								const Eigen::Vector3f& b)
	{
		for (int i = 0; i < 3; ++i) {
			Ng[i] = n(i);
			Nd[i] = n(i);
			Nx[i] = t(i);
			Ny[i] = b(i);
		}
	}

	inline void setUV(const Eigen::Vector2f& uv)
	{
		for (int i = 0; i < 2; ++i) {
			UVW[i]  = uv(i);
			dUVW[i] = 0;
		}
		UVW[2]  = 0;
		dUVW[2] = 0;
	}
};
} // namespace PR
