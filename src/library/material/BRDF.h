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
		static inline float orennayar(float roughness,
			const PM::vec3 V, const PM::vec3 N, const PM::vec3 L,
			float NdotV, float NdotL);
		
		// NDF
		static inline float ndf_blinn(float NdotH, float alpha);
		static inline float ndf_beckmann(float NdotH, float alpha);
		static inline float ndf_ggx_iso(float NdotH, float alpha);
		static inline float ndf_ggx_aniso(float NdotH, float XDotH, float YdotH, float alphaX, float alphaY);

		// Geometry (includes NdotL*NdotV term)
		static inline float g_implicit(float NdotV, float NdotL);
		static inline float g_neumann(float NdotV, float NdotL);
		static inline float g_cooktorrance(float NdotV, float NdotL, float NdotH, float VdotH);
		static inline float g_kelemen(float NdotV, float NdotL, float VdotH);

		// Smith etc
	};
}

#include "BRDF.inl"