#include "math/Microfacet.h"
#include "shader/INode.h"

namespace PR {

/// Helping class for GGX based microfacets
/// As this is as generic as possible the reflection or refraction jacobian has to be applied
// TODO: VNDF does not work
template <bool IsAnisotropic, bool UseVNDF, bool SquareRoughness = true>
class Roughness {
public:
	/// Closure environment in which roughness is already evaluated
	struct Closure {
		const float M1;
		const float M2;

		inline Closure(float m1, float m2)
			: M1(m1)
			, M2(m2)
		{
		}

		inline bool isDelta() const { return M1 <= PR_EPSILON || M2 <= PR_EPSILON; }

		inline float G(const ShadingVector& V, const ShadingVector& L) const
		{
			if constexpr (!IsAnisotropic)
				return Microfacet::g_1_smith(V, M1) * Microfacet::g_1_smith(L, M1);
			else
				return Microfacet::g_1_smith(V, M1, M2) * Microfacet::g_1_smith(L, M1, M2);
		}

		inline float D(const ShadingVector& H) const
		{
			if constexpr (!IsAnisotropic)
				return Microfacet::ndf_ggx(H.absCosTheta(), M1);
			else
				return Microfacet::ndf_ggx(H, M1, M2);
		}

		/// Calculate jacobian for mapping from H to N, in which the cosine term is already applied
		inline float Norm(const ShadingVector& H, const ShadingVector& V) const
		{
			const float denom = V.absCosTheta();
			if (denom <= PR_EPSILON)
				return 0;

			const float HdotV = H.dot(V); // By definition this is also HdotL only if this is based on a reflection
			return std::abs(HdotV) / denom;
		}

		inline float GNorm(const ShadingVector& H, const ShadingVector& V, const ShadingVector& L) const
		{
			return G(V, L) * Norm(H, V);
		}

		inline float DGNorm(const ShadingVector& H, const ShadingVector& V, const ShadingVector& L) const
		{
			return D(H) * G(V, L) * Norm(H, V);
		}

		inline float DG(const ShadingVector& H, const ShadingVector& V, const ShadingVector& L) const
		{
			return D(H) * G(V, L);
		}

		inline float pdf(const ShadingVector& H, const ShadingVector& V) const
		{
			if (isDelta())
				return 1.0f;

			if constexpr (UseVNDF) {
				return Microfacet::pdf_ggx_vndf(V, H, M1, M2);
			} else {
				if constexpr (!IsAnisotropic)
					return Microfacet::pdf_ggx(H.absCosTheta(), M1);
				else
					return Microfacet::pdf_ggx(H, M1, M2);
			}
		}

		inline Vector3f sample(const Vector2f& rnd, const ShadingVector& V) const
		{
			if (isDelta())
				return Vector3f(0, 0, 1);

			if constexpr (UseVNDF) {
				return Microfacet::sample_vndf_ggx(rnd[0], rnd[1], V, M1, M2);
			} else {
				if constexpr (IsAnisotropic) {
					return Microfacet::sample_ndf_ggx(rnd[0], rnd[1], M1, M2);
				} else {
					return Microfacet::sample_ndf_ggx(rnd[0], rnd[1], M1);
				}
			}
		}
	};

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

	inline Closure closure(const ShadingContext& sctx) const
	{
		const auto [m1, m2] = evalRoughness(sctx);
		return Closure(m1, m2);
	}

	inline std::shared_ptr<FloatScalarNode> roughnessX() const { return mRoughnessX; }
	inline std::shared_ptr<FloatScalarNode> roughnessY() const { return mRoughnessY; }

private:
	const std::shared_ptr<FloatScalarNode> mRoughnessX;
	const std::shared_ptr<FloatScalarNode> mRoughnessY;
};
} // namespace PR