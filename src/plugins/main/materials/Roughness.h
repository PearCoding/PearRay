#include "math/Microfacet.h"
#include "shader/INode.h"

namespace PR {

/// Helping class for GGX based microfacets
// CHECK: The use of the jacobian for non VNDF is correct, but for VNDF?
// TODO: VNDF does not work
template <bool IsAnisotropic, bool UseVNDF, bool SquareRoughness = true>
class Roughness {
public:
	Roughness(const std::shared_ptr<FloatScalarNode>& roughnessX, const std::shared_ptr<FloatScalarNode>& roughnessY)
		: mRoughnessX(roughnessX)
		, mRoughnessY(roughnessY)
	{
		PR_ASSERT(IsAnisotropic || roughnessX == roughnessY, "If isotropic, both nodes have to be the same");
	}

	inline static float adaptR(float r)
	{
		if constexpr (SquareRoughness)
			return r * r;
		else
			return r;
	}

	inline std::pair<float, float> evalRoughness(const ShadingContext& sctx) const
	{
		const float m1 = adaptR(mRoughnessX->eval(sctx));
		if constexpr (IsAnisotropic)
			return { m1, adaptR(mRoughnessY->eval(sctx)) };
		else
			return { m1, m1 };
	}

	inline float eval(const ShadingVector& H, const ShadingVector& V, const ShadingVector& L, const ShadingContext& sctx, float& pdf) const
	{
		const float absNdotH = H.absCosTheta();
		const float absNdotV = V.absCosTheta();

		//PR_ASSERT(absNdotV >= 0, "By definition N.V has to be positive");
		if (absNdotH <= PR_EPSILON
			|| absNdotV <= PR_EPSILON) {
			pdf = 0;
			return 0.0f;
		}

		const auto [m1, m2] = evalRoughness(sctx);
		// Check for delta
		if (m1 <= PR_EPSILON || m2 <= PR_EPSILON) {
			pdf = 0.0f;
			return 0.0f;
		}

		float G;
		float D;
		if constexpr (!IsAnisotropic) {
			D = Microfacet::ndf_ggx(absNdotH, m1);
			G = Microfacet::g_1_smith(V, m1) * Microfacet::g_1_smith(L, m1);
		} else {
			D = Microfacet::ndf_ggx(H, m1, m2);
			G = Microfacet::g_1_smith(V, m1, m2) * Microfacet::g_1_smith(L, m1, m2);
		}

		const float HdotV  = H.dot(V); // By definition this is also HdotL only if this is based on a reflection
		const float HdotL  = H.dot(L);
		const float factor = std::abs(HdotV * HdotL) / absNdotV; // NdotL multiplied out
		pdf				   = std::abs(D * absNdotH);
		return G * D * factor;
	}

	inline float pdf(const ShadingVector& H, const ShadingVector& V, const ShadingContext& sctx) const
	{
		const float absNdotH = H.absCosTheta();
		const float absNdotV = V.absCosTheta();

		//PR_ASSERT(absNdotV >= 0, "By definition N.V has to be positive");
		if (absNdotH <= PR_EPSILON
			|| absNdotV <= PR_EPSILON)
			return 0.0f;

		const auto [m1, m2] = evalRoughness(sctx);
		// Check for delta
		if (m1 <= PR_EPSILON || m2 <= PR_EPSILON)
			return 0.0f;

		if constexpr (UseVNDF) {
			return Microfacet::pdf_ggx_vndf(V, H, m1, m2);
		} else {
			if constexpr (!IsAnisotropic)
				return Microfacet::pdf_ggx(absNdotH, m1);
			else
				return Microfacet::pdf_ggx(H, m1, m2);
		}
	}

	inline ShadingVector sample(const Vector2f& rnd, const ShadingVector& V, const ShadingContext& sctx, float& pdf, bool& delta) const
	{
		pdf = 1;

		const auto [m1, m2] = evalRoughness(sctx);
		delta				= m1 <= PR_EPSILON || m2 <= PR_EPSILON;
		if (delta)
			return Vector3f(0, 0, 1);

		if constexpr (UseVNDF) {
			return Microfacet::sample_vndf_ggx(rnd[0], rnd[1], V, m1, m2, pdf);
		} else {
			if constexpr (IsAnisotropic)
				return Microfacet::sample_ndf_ggx(rnd[0], rnd[1], m1, m2, pdf);
			else
				return Microfacet::sample_ndf_ggx(rnd[0], rnd[1], m1, pdf);
		}
	}

	inline std::shared_ptr<FloatScalarNode> roughnessX() const { return mRoughnessX; }
	inline std::shared_ptr<FloatScalarNode> roughnessY() const { return mRoughnessY; }

private:
	const std::shared_ptr<FloatScalarNode> mRoughnessX;
	const std::shared_ptr<FloatScalarNode> mRoughnessY;
};
} // namespace PR