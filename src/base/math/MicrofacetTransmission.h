#pragma once

#include "math/Fresnel.h"
#include "math/RoughDistribution.h"

namespace PR {

/// Helping class for GGX based transmission
/// The cosine term is already included
template <bool IsAnisotropic, bool UseVNDF>
class PR_LIB_BASE MicrofacetTransmission {
public:
	using Distr = RoughDistribution<IsAnisotropic, UseVNDF>;
	const Distr Distribution;
	const float InnerIOR;
	const float OuterIOR;

	inline MicrofacetTransmission(float m1, float m2, float inner_ior, float outer_ior)
		: Distribution(m1, m2)
		, InnerIOR(inner_ior)
		, OuterIOR(outer_ior)
	{
	}

	inline MicrofacetTransmission(const Distr& distr, float inner_ior, float outer_ior)
		: Distribution(distr)
		, InnerIOR(inner_ior)
		, OuterIOR(outer_ior)
	{
	}

	inline bool isDelta() const { return Distribution.isDelta(); }

	inline float evalDielectric(const ShadingVector& wIn, const ShadingVector& wOut, bool isLightPath = false) const
	{
		// No reflection
		if (wIn.sameHemisphere(wOut))
			return 0.0f;

		const float in_ior	= wIn.isPositiveHemisphere() ? InnerIOR : OuterIOR;
		const float out_ior = wIn.isPositiveHemisphere() ? OuterIOR : InnerIOR;

		ShadingVector H = Scattering::halfway_refractive(in_ior, wIn, out_ior, wOut);
		if (!H.isPositiveHemisphere())
			H = -H;

		const float cosI = H.dot(wIn);
		const float cosO = H.dot(wOut);
		// Check if halfway vector configuration is correct
		if (cosI * cosO >= -PR_EPSILON)
			return 0.0f;

		const float F = Fresnel::dielectric(cosI, InnerIOR, OuterIOR);

		if (isDelta())
			return 1 - F;

		const float eta		 = in_ior / out_ior;
		const float jacobian = Scattering::refractive_jacobian(eta, cosI, cosO);
		const float spread	 = isLightPath ? 1 / (eta * eta) : 1.0f; // TODO: Correct orientation
		return (1 - F) * Distribution.DGNorm(H, wIn, wOut) * jacobian * spread;
	}

	inline float eval(const ShadingVector& wIn, const ShadingVector& wOut, bool isLightPath = false) const
	{
		// No reflection
		if (wIn.sameHemisphere(wOut))
			return 0.0f;

		const float in_ior	= wIn.isPositiveHemisphere() ? InnerIOR : OuterIOR;
		const float out_ior = wIn.isPositiveHemisphere() ? OuterIOR : InnerIOR;

		ShadingVector H = Scattering::halfway_refractive(in_ior, wIn, out_ior, wOut);
		if (!H.isPositiveHemisphere())
			H = -H;

		const float cosO = H.dot(wOut);
		const float cosI = H.dot(wIn);
		// Check if halfway vector configuration is correct
		if (cosI * cosO >= -PR_EPSILON)
			return 0.0f;

		if (isDelta())
			return 1.0f;

		const float eta		 = in_ior / out_ior;
		const float jacobian = Scattering::refractive_jacobian(eta, cosI, cosO);
		const float spread	 = isLightPath ? 1 / (eta * eta) : 1.0f; // TODO: Correct orientation
		return Distribution.DGNorm(H, wIn, wOut) * jacobian * spread;
	}

	inline float pdf(const ShadingVector& wIn, const ShadingVector& wOut) const
	{
		// No reflection
		if (wIn.sameHemisphere(wOut))
			return 0.0f;

		const float in_ior	= wIn.isPositiveHemisphere() ? InnerIOR : OuterIOR;
		const float out_ior = wIn.isPositiveHemisphere() ? OuterIOR : InnerIOR;

		ShadingVector H = Scattering::halfway_refractive(in_ior, wIn, out_ior, wOut);
		if (!H.isPositiveHemisphere())
			H = -H;

		const float cosO = H.dot(wOut);
		const float cosI = H.dot(wIn);
		// Check if halfway vector configuration is correct
		if (cosI * cosO >= -PR_EPSILON)
			return 0.0f;

		if (isDelta())
			return 1.0f;

		const float eta		 = in_ior / out_ior;
		const float jacobian = Scattering::refractive_jacobian(eta, cosI, cosO);
		return Distribution.pdf(H, wIn) * jacobian;
	}

	inline Vector3f sample(const Vector2f& rnd, const ShadingVector& wIn) const
	{
		const ShadingVector H = Distribution.sample(rnd, wIn);

		// In some cases the vector may be invalid
		if (PR_UNLIKELY(H.isZero(PR_EPSILON)))
			return Vector3f::Zero();

		PR_ASSERT(H.isPositiveHemisphere(), "Microfacet sample has to be positive hemisphere by definition");

		const float eta = InnerIOR / OuterIOR;
		bool total;
		const Vector3f L = Scattering::refract(eta, wIn, H, total);

		// Check if its a valid refraction if its not a total reflection, and if its a valid reflection if its a total reflection
		if (total == wIn.sameHemisphere(L))
			return L;
		else
			return Vector3f::Zero();
	}
};
} // namespace PR