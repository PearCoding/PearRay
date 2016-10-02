namespace PR
{
	// http://graphicrants.blogspot.de/2013/08/specular-brdf-reference.html

	//inline float BRDF::fresnel_cocktorrance(float f0, const PM::vec3& L, const PM::vec3& N);

	// NDF
	inline float BRDF::ndf_blinn(float NdotH, float alpha)
	{
		const float alpha2 = 1/(alpha*alpha);
		return powf(NdotH, 2 * alpha2 - 2)*alpha2*PM_INV_PI_F;
	}

	inline float BRDF::ndf_beckmann(float NdotH, float alpha)
	{
		const float alpha2 = 1 / (alpha*alpha);
		const float dot2 = NdotH*NdotH;
		return (alpha2*PM_INV_PI_F / (dot2*dot2))*expf(alpha2 * (dot2 - 1) / dot2);
	}

	inline float BRDF::ndf_ggx(float NdotH, float alpha)
	{
		const float alpha2 = alpha*alpha;
		const float dot2 = NdotH*NdotH;
		const float inv_t = (dot2 * (alpha2 - 1) + 1);
		const float inv_t2 = inv_t * inv_t;

		return alpha2 * PM_INV_PI_F / inv_t2;
	}

	// Geometry
	inline float BRDF::g_implicit(float NdotV, float NdotL)
	{
		return NdotV * NdotL;
	}

	inline float BRDF::g_neumann(float NdotV, float NdotL)
	{
		return NdotV * NdotL / PM::pm_MaxT(NdotV, NdotL);
	}

	inline float BRDF::g_cocktorrance(float NdotV, float NdotL, float NdotH, float VdotH)
	{
		return PM::pm_MinT(1.0f, PM::pm_MinT(2 * NdotV, 2 * NdotH*NdotL) / VdotH);
	}

	inline float BRDF::g_kelemen(float NdotV, float NdotL, float VdotH)
	{
		return NdotV * NdotL / (VdotH * VdotH);
	}
}