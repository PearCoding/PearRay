#pragma once

#include "spectral/Spectrum.h"
#include "PearMath.h"

namespace PR
{
	/*
	 Utils class for the different brdf implementations
	*/
	class PR_LIB BRDF
	{
	public:
		// NDF
		static inline float ndf_blinn(float NdotH, float alpha);
		static inline float ndf_beckmann(float NdotH, float alpha);
		static inline float ndf_ggx(float NdotH, float alpha);

		// Geometry
		static inline float g_implicit(float NdotV, float NdotL);
		static inline float g_neumann(float NdotV, float NdotL);
		static inline float g_cocktorrance(float NdotV, float NdotL, float NdotH, float VdotH);
		static inline float g_kelemen(float NdotV, float NdotL, float VdotH);

		// Smith etc
	};
}

#include "BRDF.inl"