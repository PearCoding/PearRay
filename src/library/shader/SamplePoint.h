#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	enum SamplePointFlags
	{
		SPF_Inside = 0x1
	};

	/*
	   Sample context - nearly the same as for OSL
	*/
	PM_ALIGN(16)
	struct PR_LIB_INLINE SamplePoint
	{
	public:
		// Point of sample
		PM::vec3 P;
		PM::vec3 dPdX;
		PM::vec3 dPdY;
		PM::vec3 dPdZ;// Only useful for volumes.
		PM::vec3 dPdU;
		PM::vec3 dPdV;
		PM::vec3 dPdT;// Velocity of P

		// View vector looking to the surface
		PM::vec3 V;
		PM::vec3 dVdX;
		PM::vec3 dVdY;

		// Normal - Front facing
		PM::vec3 N;

		// Geometric normal.
		PM::vec3 Ng;

		// Normal Tangent Frame
		PM::vec3 Nx;
		PM::vec3 Ny;

		// 2D surface parameters
		PM::vec2 UV;
		PM::vec2 dUVdX;
		PM::vec2 dUVdY;

		// Time for this sample.
		float T;
		float dT;

		// Some other utility variables
		float NdotV;
		uint8 Flags;

		class Material* Material;

		// C++11 POD constructor
		SamplePoint() noexcept :
			P(PM::pm_Zero()),
			dPdX(PM::pm_Zero()),
			dPdY(PM::pm_Zero()),
			dPdZ(PM::pm_Zero()),
			dPdU(PM::pm_Zero()),
			dPdV(PM::pm_Zero()),
			dPdT(PM::pm_Zero()),
			V(PM::pm_Zero()),
			dVdX(PM::pm_Zero()),
			dVdY(PM::pm_Zero()),
			N(PM::pm_Zero()),
			Ng(PM::pm_Zero()),
			Nx(PM::pm_Zero()),
			Ny(PM::pm_Zero()),
			UV(PM::pm_Zero()),
			dUVdX(PM::pm_Zero()),
			dUVdY(PM::pm_Zero()),
			T(0),
			dT(0),
			NdotV(0),
			Flags(0),
			Material(nullptr)
			{}
	};
}