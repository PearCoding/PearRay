#include "BRDF.h"

namespace PR
{
	// http://graphicrants.blogspot.de/2013/08/specular-brdf-reference.html

	float BRDF::fresnel_schlick(float f0, const PM::vec3& V, const PM::vec3& N)
	{
		return f0 + (1 - f0)*fresnel_schlick_term(V, N);
	}

	float BRDF::fresnel_schlick_term(const PM::vec3& V, const PM::vec3& N)
	{
		const float t = 1 + PM::pm_Dot3D(N, V);

		return t*t*t*t*t;
	}

	//float BRDF::fresnel_cocktorrance(float f0, const PM::vec3& L, const PM::vec3& N);

	// NDF
	float BRDF::ndf_blinn(const PM::vec3& H, const PM::vec3& N, float alpha)
	{
		const float alpha2 = 1/(alpha*alpha);
		return powf(PM::pm_Dot3D(N, H), 2 * alpha2 - 2)*alpha2*PM_INV_PI_F;
	}

	float BRDF::ndf_beckmann(const PM::vec3& H, const PM::vec3& N, float alpha)
	{
		const float alpha2 = 1 / (alpha*alpha);
		const float dot = PM::pm_Dot3D(N, H);
		const float dot2 = dot*dot;
		return (alpha2*PM_INV_PI_F / (dot2*dot2))*expf(alpha2 * (dot2 - 1) / dot2);
	}

	float BRDF::ndf_ggx(const PM::vec3& H, const PM::vec3& N, float alpha)
	{
		const float alpha2 = alpha*alpha;
		const float dot = PM::pm_Dot3D(N, H);
		const float dot2 = dot*dot;
		const float inv_t = (dot2 * (alpha2 - 1) + 1);
		const float inv_t2 = inv_t * inv_t;

		return alpha2 * PM_INV_PI_F / inv_t2;
	}

	// Geometry
	float BRDF::g_implicit(const PM::vec3& L, const PM::vec3& V, const PM::vec3& H, const PM::vec3& N)
	{
		return PM::pm_Dot3D(N, L) * PM::pm_Dot3D(N, V);
	}

	float BRDF::g_neumann(const PM::vec3& L, const PM::vec3& V, const PM::vec3& H, const PM::vec3& N)
	{
		const float dot1 = PM::pm_Dot3D(N, L);
		const float dot2 = PM::pm_Dot3D(N, V);
		return dot1 * dot2 / PM::pm_MaxT(dot1, dot2);
	}

	float BRDF::g_cocktorrance(const PM::vec3& L, const PM::vec3& V, const PM::vec3& H, const PM::vec3& N)
	{
		const float dot1 = PM::pm_Dot3D(N, H);
		const float dot2 = PM::pm_Dot3D(V, H);

		return PM::pm_MinT(1.0f, PM::pm_MinT(2 * dot1*PM::pm_Dot3D(N, V), 2 * dot1*PM::pm_Dot3D(N, L)) / dot2);
	}

	float BRDF::g_kelemen(const PM::vec3& L, const PM::vec3& V, const PM::vec3& H, const PM::vec3& N)
	{
		const float dot = PM::pm_Dot3D(V, H);
		return PM::pm_Dot3D(N, L) * PM::pm_Dot3D(N, V) / (dot * dot);
	}

	// Smith etc

	// Optimized groups:

	// Fresnel: Schlick, NDF: Beckmann
	// See http://blog.selfshadow.com/publications/s2013-shading-course/ for more information.
	float BRDF::standard(float f0, float alpha, const PM::vec3& L, const PM::vec3& N, const PM::vec3& H, const PM::vec3& V)
	{
		// Only optimizing is in the geometry
		float fresnel = fresnel_schlick(f0, L, H);
		float geometry = PM::pm_Dot3D(N, H) / (PM::pm_Dot3D(N, L)*PM::pm_Dot3D(V, H));
		float ndf = ndf_beckmann(H, N, alpha);

		return fresnel * geometry * ndf * 0.25f;
	}
}