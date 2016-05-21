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
		/**
		 * Reflects the viewing vector through the surface normal.
		 * L = V - 2(N*V)N
		 *
		 * @param N Normal of the surface point.
		 * @param V Unit vector pointing TO the surface point.
		 * @return Unit vector pointing FROM the surface point outwards.
		 */
		inline static PM::vec3 reflect(const PM::vec3& N, const PM::vec3& V)
		{
			return PM::pm_Subtract(V, PM::pm_Scale(N, 2 * PM::pm_Dot3D(V, N)));
		}

		/**
		 * Refracts the ray based on the r parameter (r = n1/n2)
		 *
		 * @param r Index ratio (n1/n2) between the two mediums.
		 * @param N Normal of the surface point.
		 * @param V Unit vector pointing TO the surface point.
		 * @return Unit vector pointing FROM the surface point outwards.
		 */
		inline static PM::vec3 refract(float r, const PM::vec3& N, const PM::vec3& V)
		{
			const float c = -PM::pm_Dot3D(V, N);
			const float t = r * std::abs(c) - std::sqrt(1 - r*r*(1 - c*c));

			PR_DEBUG_ASSERT(c >= 0);// Should be handled by upper code.

			return PM::pm_Normalize3D(PM::pm_Add(PM::pm_Scale(V, r), PM::pm_Scale(N, t)));
		}

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