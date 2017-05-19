namespace PR
{
	inline float BRDF::orennayar(float roughness,
		const Eigen::Vector3f V, const Eigen::Vector3f N, const Eigen::Vector3f L,
		float NdotV, float NdotL)
	{
		const float angleVN = std::cos(-NdotV);
		const float angleLN = std::cos(NdotL);
		const float or_alpha = std::max(angleLN, angleVN);
		const float or_beta = std::min(angleLN, angleVN);

		const float A = 1 - 0.5f * roughness / (roughness + 0.57f);
		const float B = 0.45f * roughness / (roughness + 0.09f);
		const float C = std::sin(or_alpha) * std::tan(or_beta);

		const float gamma = (V - N*NdotV).dot(L - N*NdotL);

		const float L1 = (A + B * C * std::max(0.0f, gamma));

		return PR_1_PI * L1;
	}

	// http://graphicrants.blogspot.de/2013/08/specular-brdf-reference.html

	// NDF
	inline float BRDF::ndf_blinn(float NdotH, float alpha)
	{
		const float alpha2 = 1 / (alpha*alpha);
		return std::pow(NdotH, 2 * alpha2 - 2)*alpha2*PR_1_PI;
	}

	inline float BRDF::ndf_beckmann(float NdotH, float alpha)
	{
		const float alpha2 = 1 / (alpha*alpha);
		const float dot2 = NdotH*NdotH;
		return (alpha2*PR_1_PI / (dot2*dot2))*std::exp(alpha2 * (dot2 - 1) / dot2);
	}

	inline float BRDF::ndf_ggx_iso(float NdotH, float alpha)
	{
		const float alpha2 = alpha*alpha;
		const float dot2 = NdotH*NdotH;
		const float inv = dot2 * (alpha2 - 1) + 1;
		const float inv_t2 = inv * inv;

		return alpha2 * PR_1_PI / inv_t2;
	}

	inline float BRDF::ndf_ggx_aniso(float NdotH, float XdotH, float YdotH, float alphaX, float alphaY)
	{
		float t = XdotH*XdotH/(alphaX*alphaX) + YdotH*YdotH/(alphaY*alphaY) + NdotH*NdotH;
		return PR_1_PI / (alphaX * alphaY * t * t);
	}

	// Geometry
	inline float BRDF::g_implicit(float dot, float NdotL)
	{
		return 1;//dot * NdotL;
	}

	inline float BRDF::g_neumann(float dot, float NdotL)
	{
		return 1/*dot * NdotL*/ / std::max(dot, NdotL);
	}

	inline float BRDF::g_cooktorrance(float dot, float NdotL, float NdotH, float VdotH)
	{
		return std::min(std::max(2 * std::min(dot, 2 * NdotH*NdotL) / (VdotH*NdotL*dot), 0.0f), 1.0f);
			//PM::pm_ClampT(PM::pm_MinT(2 * dot, 2 * NdotH*NdotL) / VdotH, 0.0f, 1.0f);
	}

	inline float BRDF::g_kelemen(float dot, float NdotL, float VdotH)
	{
		return 1/*dot * NdotL*/ / (VdotH * VdotH);
	}
}
