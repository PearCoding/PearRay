#pragma once

#include "math/RoughDistribution.h"
#include "math/Fresnel.h"

namespace PR {

/// Helping class for GGX based reflection
/// The cosine term is already included
/// This is [F *] G * D / ( 4 * N.V ). The 1/N.L is out already due to being multiplied by N.L
template <bool IsAnisotropic, bool UseVNDF>
class PR_LIB_BASE MicrofacetReflection {
public:
	using Distr = RoughDistribution<IsAnisotropic, UseVNDF>;
	const Distr Distribution;

	inline MicrofacetReflection(float m1, float m2)
		: Distribution(m1, m2)
	{
	}

	inline MicrofacetReflection(const Distr& distr)
		: Distribution(distr)
	{
	}

	inline bool isDelta() const { return Distribution.isDelta(); }

	/// Evaluate for dielectrics
	inline float evalDielectric(const ShadingVector& wIn, const ShadingVector& wOut, float n_in, float n_out) const
	{
		// No transmission
		if (!wIn.sameHemisphere(wOut))
			return 0.0f;

		PR_ASSERT(n_in > PR_EPSILON && n_out > PR_EPSILON, "Expected valid IOR when evaluation with Fresnel term");

		ShadingVector H = Scattering::halfway_reflection(wIn, wOut);
		if (!H.isPositiveHemisphere())
			H = -H;

		const float cosI = H.dot(wIn); // Same as cosO
		const float F	 = Fresnel::dielectric(cosI, n_in, n_out);
		if (isDelta())
			return F;

		const float jacobian = Scattering::reflective_jacobian(cosI);
		return F * Distribution.DGNorm(H, wIn, wOut) * jacobian;
	}

	/// Evaluate for conductors
	inline float evalConductor(const ShadingVector& wIn, const ShadingVector& wOut, float ior, float kappa) const
	{
		// No transmission
		if (!wIn.sameHemisphere(wOut))
			return 0.0f;

		PR_ASSERT(ior > PR_EPSILON, "Expected valid IOR when evaluation with Fresnel term");
		PR_ASSERT(kappa >= 0.0f, "Expected valid absorption term when evaluation with Fresnel term");

		ShadingVector H = Scattering::halfway_reflection(wIn, wOut);
		if (!H.isPositiveHemisphere())
			H = -H;

		const float cosI = H.dot(wIn); // Same as cosO
		const float F	 = Fresnel::conductor(cosI, 1, ior, kappa);
		if (isDelta())
			return F;

		const float jacobian = Scattering::reflective_jacobian(cosI);
		return F * Distribution.DGNorm(H, wIn, wOut) * jacobian;
	}

	/// Evaluate without fresnel term
	inline float eval(const ShadingVector& wIn, const ShadingVector& wOut) const
	{
		// No transmission
		if (!wIn.sameHemisphere(wOut))
			return 0.0f;

		const ShadingVector H = Scattering::halfway_reflection(wIn, wOut);
		if (isDelta())
			return 1.0f;

		const float cosI	 = H.dot(wIn); // Same as cosO
		const float jacobian = Scattering::reflective_jacobian(cosI);
		return Distribution.DGNorm(H, wIn, wOut) * jacobian;
	}

	inline float pdf(const ShadingVector& wIn, const ShadingVector& wOut) const
	{
		// No transmission
		if (!wIn.sameHemisphere(wOut))
			return 0.0f;

		const ShadingVector H = Scattering::halfway_reflection(wIn, wOut);
		if (isDelta())
			return 1.0f;

		const float cosI	 = H.dot(wIn); // Same as cosO
		const float jacobian = Scattering::reflective_jacobian(cosI);
		return jacobian * Distribution.pdf(H, wIn);
	}

	inline Vector3f sample(const Vector2f& rnd, const ShadingVector& wIn) const
	{
		const ShadingVector H = Distribution.sample(rnd, wIn);

		// In some cases the vector may be invalid
		if (PR_UNLIKELY(H.isZero(PR_EPSILON)))
			return Vector3f::Zero();

		PR_ASSERT(H.isPositiveHemisphere(), "Microfacet sample has to be positive hemisphere by definition");

		const Vector3f wOut = Scattering::reflect(wIn, H);
		return wIn.sameHemisphere(wOut) ? wOut : Vector3f::Zero();
	}
};
} // namespace PR