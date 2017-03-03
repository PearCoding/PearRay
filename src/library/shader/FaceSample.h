#pragma once

#include "PR_Config.h"
#include "PearMath.h"

namespace PR
{
	/*
	   Sample context
	   View independent!
	*/
	struct alignas(16) PR_LIB_INLINE FaceSample
	{
	public:
		// Point of sample
		PM::vec3 P;
		PM::vec3 dPdX;
		PM::vec3 dPdY;
		PM::vec3 dPdZ;// Only useful for volumes.
		PM::vec3 dPdU;
		PM::vec3 dPdV;
		PM::vec3 dPdW;
		PM::vec3 dPdT;// Velocity of P

		// Geometric normal.
		PM::vec3 Ng;
		// Normal Tangent Frame
		PM::vec3 Nx;
		PM::vec3 Ny;

		// 3D surface parameters
		PM::vec3 UVW;
		PM::vec3 dUVWdX;
		PM::vec3 dUVWdY;
		PM::vec3 dUVWdZ;

		class Material* Material;

		// C++11 POD constructor
		inline FaceSample() noexcept :
			P(PM::pm_Zero3D()),
			dPdX(PM::pm_Zero3D()),
			dPdY(PM::pm_Zero3D()),
			dPdZ(PM::pm_Zero3D()),
			dPdU(PM::pm_Zero3D()),
			dPdV(PM::pm_Zero3D()),
			dPdW(PM::pm_Zero3D()),
			dPdT(PM::pm_Zero3D()),
			Ng(PM::pm_Zero3D()),
			Nx(PM::pm_Zero3D()),
			Ny(PM::pm_Zero3D()),
			UVW(PM::pm_Zero3D()),
			dUVWdX(PM::pm_Zero3D()),
			dUVWdY(PM::pm_Zero3D()),
			dUVWdZ(PM::pm_Zero3D()),
			Material(nullptr)
			{}
	};
}
