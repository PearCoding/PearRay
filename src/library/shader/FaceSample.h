#pragma once

#include "PR_Config.h"
#include <Eigen/Dense>

namespace PR {
/*
	   Sample context
	   View independent!
	*/
struct alignas(16) PR_LIB_INLINE FaceSample {
public:
	// Point of sample
	Eigen::Vector3f P;
	Eigen::Vector3f dPdX;
	Eigen::Vector3f dPdY;
	Eigen::Vector3f dPdZ; // Only useful for volumes.
	Eigen::Vector3f dPdU;
	Eigen::Vector3f dPdV;
	Eigen::Vector3f dPdW;
	Eigen::Vector3f dPdT; // Velocity of P

	// Geometric normal.
	Eigen::Vector3f Ng;
	// Normal Tangent Frame
	Eigen::Vector3f Nx;
	Eigen::Vector3f Ny;

	// 3D surface parameters
	Eigen::Vector3f UVW;
	Eigen::Vector3f dUVWdX;
	Eigen::Vector3f dUVWdY;
	Eigen::Vector3f dUVWdZ;

	class Material* Material;

	// C++11 POD constructor
	inline FaceSample() noexcept
		: P(0, 0, 0)
		, dPdX(0, 0, 0)
		, dPdY(0, 0, 0)
		, dPdZ(0, 0, 0)
		, dPdU(0, 0, 0)
		, dPdV(0, 0, 0)
		, dPdW(0, 0, 0)
		, dPdT(0, 0, 0)
		, Ng(0, 0, 0)
		, Nx(0, 0, 0)
		, Ny(0, 0, 0)
		, UVW(0, 0, 0)
		, dUVWdX(0, 0, 0)
		, dUVWdY(0, 0, 0)
		, dUVWdZ(0, 0, 0)
		, Material(nullptr)
	{
	}

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};
}
