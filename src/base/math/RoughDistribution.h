#pragma once

#include "math/Microfacet.h"

namespace PR {
/// Helping class for GGX based microfacets
/// As this is as generic as possible the reflection or refraction jacobian has to be applied
// TODO: VNDF does not work
// TODO: Add energy conservation method, maybe: "Practical multiple scattering compensation for microfacet models" by Emmanuel Turquin?
template <bool IsAnisotropic, bool UseVNDF>
class PR_LIB_BASE RoughDistribution {
public:
	const float M1;
	const float M2;

	inline RoughDistribution(float m1, float m2)
		: M1(m1)
		, M2(m2)
	{
	}

	inline bool isDelta() const
	{
		constexpr float EPS = 1e-3f;
		return M1 <= EPS || M2 <= EPS;
	}

	inline float G(const ShadingVector& H, const ShadingVector& V, const ShadingVector& L) const
	{
		// Characteristic functions
		const bool chi_v = V.cosTheta() * H.dot(V) > PR_EPSILON;
		const bool chi_l = L.cosTheta() * H.dot(L) > PR_EPSILON;

		if (!chi_v || !chi_l)
			return 0.0f;

		if constexpr (!UseVNDF) {
			if constexpr (!IsAnisotropic) {
				return Microfacet::g_1_smith(V, M1) * Microfacet::g_1_smith(L, M1);
			} else {
				return Microfacet::g_1_smith(V, M1, M2) * Microfacet::g_1_smith(L, M1, M2);
			}
		} else {
			if constexpr (!IsAnisotropic) {
				const float denom = 1 + Microfacet::g_1_smith_lambda(V, M1) + Microfacet::g_1_smith_lambda(L, M1);
				return denom <= PR_EPSILON ? 0.0f : 1 / denom;
			} else {
				const float denom = 1 + Microfacet::g_1_smith_lambda(V, M1, M2) + Microfacet::g_1_smith_lambda(L, M1, M2);
				return denom <= PR_EPSILON ? 0.0f : 1 / denom;
			}
		}
	}

	inline float D(const ShadingVector& H) const
	{
		if constexpr (!IsAnisotropic)
			return Microfacet::ndf_ggx(H, M1);
		else
			return Microfacet::ndf_ggx(H, M1, M2);
	}

	/// Calculate jacobian for mapping from H to N,
	/// The cosine term and the inverse outgoing cosine term (of specular bsdfs) are already applied
	inline float Norm(const ShadingVector& H, const ShadingVector& V, const ShadingVector& L) const
	{
		const float denom = V.absCosTheta();
		if (denom <= PR_EPSILON)
			return 0;

		const float HdotL = H.dot(L); // By definition this is HdotV only if it is a reflective halfway vector
		return std::abs(HdotL) / denom;
	}

	inline float GNorm(const ShadingVector& H, const ShadingVector& V, const ShadingVector& L) const
	{
		return G(V, L) * Norm(H, V, L);
	}

	inline float DGNorm(const ShadingVector& H, const ShadingVector& V, const ShadingVector& L) const
	{
		return D(H) * G(H, V, L) * Norm(H, V, L);
	}

	inline float DG(const ShadingVector& H, const ShadingVector& V, const ShadingVector& L) const
	{
		return D(H) * G(H, V, L);
	}

	inline float pdf(const ShadingVector& H, const ShadingVector& V) const
	{
		if (isDelta())
			return 1.0f;

		if constexpr (UseVNDF) {
			return Microfacet::pdf_ggx_vndf(V.makePositiveHemisphere(), H.makePositiveHemisphere(), M1, M2);
		} else {
			if constexpr (!IsAnisotropic)
				return Microfacet::pdf_ggx(H, M1);
			else
				return Microfacet::pdf_ggx(H, M1, M2);
		}
	}

	inline Vector3f sample(const Vector2f& rnd, const ShadingVector& V) const
	{
		if (isDelta())
			return Vector3f(0, 0, 1);

		if constexpr (UseVNDF) {
			return Microfacet::sample_vndf_ggx(rnd[0], rnd[1], V.makePositiveHemisphere(), M1, M2);
		} else {
			if constexpr (IsAnisotropic) {
				return Microfacet::sample_ndf_ggx(rnd[0], rnd[1], M1, M2);
			} else {
				return Microfacet::sample_ndf_ggx(rnd[0], rnd[1], M1);
			}
		}
	}
};
} // namespace PR