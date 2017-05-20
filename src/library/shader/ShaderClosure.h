#pragma once

#include "FaceSample.h"

namespace PR
{
	enum ShaderClosureFlags
	{
		SCF_Inside = 0x1
	};

	/*
	   Sample context - nearly the same as for OSL
	   View dependent!
	*/
	struct alignas(16) PR_LIB_INLINE ShaderClosure
	{
	public:
		// Point of sample
		Eigen::Vector3f P;
		Eigen::Vector3f dPdX;
		Eigen::Vector3f dPdY;
		Eigen::Vector3f dPdZ;// Only useful for volumes.
		Eigen::Vector3f dPdU;
		Eigen::Vector3f dPdV;
		Eigen::Vector3f dPdW;
		Eigen::Vector3f dPdT;// Velocity of P

		// View vector looking to the surface (only available if shoot)
		Eigen::Vector3f V;
		Eigen::Vector3f dVdX;
		Eigen::Vector3f dVdY;

		// Normal - Front facing
		Eigen::Vector3f N;

		// Geometric normal.
		Eigen::Vector3f Ng;

		// Normal Tangent Frame
		Eigen::Vector3f Nx;
		Eigen::Vector3f Ny;

		// 2D surface parameters
		Eigen::Vector3f UVW;
		Eigen::Vector3f dUVWdX;
		Eigen::Vector3f dUVWdY;
		Eigen::Vector3f dUVWdZ;

		// Time for this sample.
		float T;
		//float dT;

		// Wavelength
		uint8 WavelengthIndex;

		// Some other utility variables
		float Depth2;// Squared!
		float NgdotV;
		float NdotV;
		uint8 Flags;

		uint32 EntityID;

		class Material* Material;

		// C++11 POD constructor
		inline ShaderClosure() noexcept :
			Material(nullptr)
			{}

		inline ShaderClosure(const FaceSample& fs) noexcept :
			P(fs.P),
			dPdX(fs.dPdX),
			dPdY(fs.dPdY),
			dPdZ(fs.dPdZ),
			dPdU(fs.dPdU),
			dPdV(fs.dPdV),
			dPdW(fs.dPdW),
			dPdT(fs.dPdT),
			V(0,0,0),
			dVdX(0,0,0),
			dVdY(0,0,0),
			N(fs.Ng),
			Ng(fs.Ng),
			Nx(fs.Nx),
			Ny(fs.Ny),
			UVW(fs.UVW),
			dUVWdX(fs.dUVWdX),
			dUVWdY(fs.dUVWdY),
			dUVWdZ(fs.dUVWdZ),
			T(0),
			//dT(0),
			WavelengthIndex(0),
			Depth2(0),
			NgdotV(0),
			NdotV(0),
			Flags(0),
			EntityID(0),
			Material(fs.Material)
			{}

		ShaderClosure& operator =(const FaceSample& fs)
		{
			P = fs.P;
			dPdX = fs.dPdX;
			dPdY = fs.dPdY;
			dPdZ = fs.dPdZ;
			dPdU = fs.dPdU;
			dPdV = fs.dPdV;
			dPdW = fs.dPdW;
			dPdT = fs.dPdT;
			N = fs.Ng;
			Ng = fs.Ng;
			Nx = fs.Nx;
			Ny = fs.Ny;
			UVW = fs.UVW;
			dUVWdX = fs.dUVWdX;
			dUVWdY = fs.dUVWdY;
			dUVWdY = fs.dUVWdY;
			Material = fs.Material;

			return *this;
		}

		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};
}
