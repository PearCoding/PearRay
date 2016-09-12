#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	/*
	   Sample context
	   View independent!
	*/
	struct PM_ALIGN(16) PR_LIB_INLINE FaceSample
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

		// Geometric normal.
		PM::vec3 Ng;
		// Normal Tangent Frame
		PM::vec3 Nx;
		PM::vec3 Ny;

		// 2D surface parameters
		PM::vec2 UV;
		PM::vec2 dUVdX;
		PM::vec2 dUVdY;

		class Material* Material;

		// C++11 POD constructor
		inline FaceSample() noexcept :
			P(PM::pm_Zero()),
			dPdX(PM::pm_Zero()),
			dPdY(PM::pm_Zero()),
			dPdZ(PM::pm_Zero()),
			dPdU(PM::pm_Zero()),
			dPdV(PM::pm_Zero()),
			dPdT(PM::pm_Zero()),
			Ng(PM::pm_Zero()),
			Nx(PM::pm_Zero()),
			Ny(PM::pm_Zero()),
			UV(PM::pm_Zero()),
			dUVdX(PM::pm_Zero()),
			dUVdY(PM::pm_Zero()),
			Material(nullptr)
			{}
	};
}