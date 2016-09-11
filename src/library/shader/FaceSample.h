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
		// Geometric normal.
		PM::vec3 Ng;
		// Normal Tangent Frame
		PM::vec3 Nx;
		PM::vec3 Ny;

		// 2D surface parameters
		PM::vec2 UV;

		class Material* Material;

		// C++11 POD constructor
		inline FaceSample() noexcept :
			P(PM::pm_Zero()),
			Ng(PM::pm_Zero()),
			Nx(PM::pm_Zero()),
			Ny(PM::pm_Zero()),
			UV(PM::pm_Zero()),
			Material(nullptr)
			{}
	};
}