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
		// Fresnel
		static float fresnel_schlick(float f0, const PM::vec3& V, const PM::vec3& N);
		static float fresnel_schlick_term(const PM::vec3& V, const PM::vec3& N);// Only the cos term
		//static float fresnel_cocktorrance(float f0, const PM::vec3& L, const PM::vec3& N);

		// NDF
		static float ndf_blinn(const PM::vec3& H, const PM::vec3& N, float alpha);
		static float ndf_beckmann(const PM::vec3& H, const PM::vec3& N, float alpha);
		static float ndf_ggx(const PM::vec3& H, const PM::vec3& N, float alpha);

		// Geometry
		static float g_implicit(const PM::vec3& L, const PM::vec3& V, const PM::vec3& H, const PM::vec3& N);
		static float g_neumann(const PM::vec3& L, const PM::vec3& V, const PM::vec3& H, const PM::vec3& N);
		static float g_cocktorrance(const PM::vec3& L, const PM::vec3& V, const PM::vec3& H, const PM::vec3& N);
		static float g_kelemen(const PM::vec3& L, const PM::vec3& V, const PM::vec3& H, const PM::vec3& N);

		// Smith etc

		// Optimized groups:

		// Fresnel: Schlick, NDF: Blinn, Geometry: Neumann
		static float standard(float f0, float alpha, const PM::vec3& L, const PM::vec3& N, const PM::vec3& H, const PM::vec3& V);
	};
}