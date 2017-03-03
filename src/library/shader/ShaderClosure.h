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
		PM::vec3 P;
		PM::vec3 dPdX;
		PM::vec3 dPdY;
		PM::vec3 dPdZ;// Only useful for volumes.
		PM::vec3 dPdU;
		PM::vec3 dPdV;
		PM::vec3 dPdW;
		PM::vec3 dPdT;// Velocity of P

		// View vector looking to the surface (only available if shoot)
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
		PM::vec3 UVW;
		PM::vec3 dUVWdX;
		PM::vec3 dUVWdY;
		PM::vec3 dUVWdZ;

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
			P(PM::pm_Zero3D()),
			dPdX(PM::pm_Zero3D()),
			dPdY(PM::pm_Zero3D()),
			dPdZ(PM::pm_Zero3D()),
			dPdU(PM::pm_Zero3D()),
			dPdV(PM::pm_Zero3D()),
			dPdW(PM::pm_Zero3D()),
			dPdT(PM::pm_Zero3D()),
			V(PM::pm_Zero3D()),
			dVdX(PM::pm_Zero3D()),
			dVdY(PM::pm_Zero3D()),
			N(PM::pm_Zero3D()),
			Ng(PM::pm_Zero3D()),
			Nx(PM::pm_Zero3D()),
			Ny(PM::pm_Zero3D()),
			UVW(PM::pm_Zero3D()),
			dUVWdX(PM::pm_Zero3D()),
			dUVWdY(PM::pm_Zero3D()),
			dUVWdZ(PM::pm_Zero3D()),
			T(0),
			//dT(0),
			WavelengthIndex(0),
			Depth2(0),
			NgdotV(0),
			NdotV(0),
			Flags(0),
			EntityID(0),
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
			V(PM::pm_Zero3D()),
			dVdX(PM::pm_Zero3D()),
			dVdY(PM::pm_Zero3D()),
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
	};
}
