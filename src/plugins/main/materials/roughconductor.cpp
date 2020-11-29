#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Microfacet.h"
#include "math/Projection.h"
#include "math/Scattering.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {
constexpr bool SQUARE_ROUGHNESS = true;
inline static float adaptR(float r)
{
	if constexpr (SQUARE_ROUGHNESS)
		return r * r;
	else
		return r;
}

// TODO: Use VNDF?
template <bool IsAnisotropic>
class RoughConductorMaterial : public IMaterial {
public:
	RoughConductorMaterial(const std::shared_ptr<FloatSpectralNode>& eta, const std::shared_ptr<FloatSpectralNode>& k,
						   const std::shared_ptr<FloatSpectralNode>& spec,
						   const std::shared_ptr<FloatScalarNode>& roughnessX,
						   const std::shared_ptr<FloatScalarNode>& roughnessY)
		: IMaterial()
		, mEta(eta)
		, mK(k)
		, mSpecularity(spec)
		, mRoughnessX(roughnessX)
		, mRoughnessY(roughnessY)
	{
	}

	virtual ~RoughConductorMaterial() = default;

	inline float evalGD(const ShadingVector& H, const ShadingVector& V, const ShadingVector& L, const ShadingContext& sctx, float& pdf) const
	{
		const float absNdotH = H.absCosTheta();
		const float absNdotV = V.absCosTheta();

		//PR_ASSERT(absNdotV >= 0, "By definition N.V has to be positive");
		if (absNdotH <= PR_EPSILON
			|| absNdotV <= PR_EPSILON) {
			pdf = 0;
			return 0.0f;
		}

		const float m1 = adaptR(mRoughnessX->eval(sctx));
		if (m1 < PR_EPSILON) {
			pdf = 1;
			return 1;
		}

		float G;
		float D;
		if constexpr (!IsAnisotropic) {
			D = Microfacet::ndf_ggx(absNdotH, m1);
			G = Microfacet::g_1_smith(V, m1) * Microfacet::g_1_smith(L, m1);
		} else {
			const float m2 = adaptR(mRoughnessY->eval(sctx));
			if (m2 < PR_EPSILON) {
				pdf = 1;
				return 1;
			}
			D = Microfacet::ndf_ggx(H, m1, m2);
			G = Microfacet::g_1_smith(V, m1, m2) * Microfacet::g_1_smith(L, m1, m2);
		}

		const float factor = std::abs(H.dot(V) * H.dot(L)) / absNdotV; // NdotL multiplied out
		pdf				   = std::abs(D * absNdotH);
		return G * D * factor;
	}

	inline float pdfGD(const ShadingVector& H, const ShadingVector& V, const ShadingContext& sctx) const
	{
		const float absNdotH = H.absCosTheta();
		const float absNdotV = V.absCosTheta();

		//PR_ASSERT(absNdotV >= 0, "By definition N.V has to be positive");
		if (absNdotH <= PR_EPSILON
			|| absNdotV <= PR_EPSILON)
			return 0.0f;

		const float m1 = adaptR(mRoughnessX->eval(sctx));
		if (m1 < PR_EPSILON)
			return 1;

		float D;
		if constexpr (!IsAnisotropic) {
			D = Microfacet::ndf_ggx(absNdotH, m1);
		} else {
			const float m2 = adaptR(mRoughnessY->eval(sctx));
			if (m2 < PR_EPSILON)
				return 1;
			D = Microfacet::ndf_ggx(H, m1, m2);
		}

		return std::abs(D * absNdotH);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;
		out.Type = MST_SpecularReflection;

		if (!in.Context.V.sameHemisphere(in.Context.L)) {
			out.PDF_S  = 0.0f;
			out.Weight = SpectralBlob::Zero();
			return;
		}

		const ShadingVector H = Scattering::halfway_reflection(in.Context.V, in.Context.L);
		const float HdotV	  = H.dot(in.Context.V);
		PR_ASSERT(HdotV >= 0.0f, "HdotV must be positive");

		float pdf;
		const float gd = evalGD(H, in.Context.V, in.Context.L, in.ShadingContext, pdf);

		const float jacobian = 1 / (4 * HdotV * HdotV);
		out.Weight			 = mSpecularity->eval(in.ShadingContext) * (gd * jacobian);
		out.PDF_S			 = pdf * jacobian;

		// Delta distributions do not allow evaluation
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		if (!in.Context.V.sameHemisphere(in.Context.L)) {
			out.PDF_S = 0;
			return;
		}

		const ShadingVector H = Scattering::halfway_reflection(in.Context.V, in.Context.L);
		const float HdotV	  = H.dot(in.Context.V);
		PR_ASSERT(HdotV >= 0.0f, "HdotV must be positive");

		const float pdf		 = pdfGD(H, in.Context.V, in.ShadingContext);
		const float jacobian = 1 / (4 * HdotV * HdotV);
		out.PDF_S			 = pdf * jacobian;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const SpectralBlob eta = mEta->eval(in.ShadingContext);
		const SpectralBlob k   = mK->eval(in.ShadingContext);

		SpectralBlob fresnel;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			fresnel[i] = Fresnel::conductor(in.Context.V.absCosTheta(), eta[i], k[i]);

		// Sample microfacet normal
		const float m1 = adaptR(mRoughnessX->eval(in.ShadingContext));
		float pdf	   = 1;
		Vector3f H;
		if constexpr (IsAnisotropic) {
			const float m2 = adaptR(mRoughnessY->eval(in.ShadingContext));
			if (m1 > PR_EPSILON && m2 > PR_EPSILON)
				H = Microfacet::sample_ndf_ggx(in.RND[0], in.RND[1], m1, m2, pdf);
			else
				H = Vector3f(0, 0, 1);
		} else {
			if (m1 > PR_EPSILON)
				H = Microfacet::sample_ndf_ggx(in.RND[0], in.RND[1], m1, pdf);
			else
				H = Vector3f(0, 0, 1);
		}

		const float HdotV = std::abs(H.dot((Vector3f)in.Context.V));
		if (HdotV <= PR_EPSILON) { 
			out = MaterialSampleOutput::Reject(MST_SpecularReflection);
			return;
		}

		// Calculate Fresnel term
		out.L = Scattering::reflect(in.Context.V, H);

		if (!in.Context.V.sameHemisphere(out.L)) { // Side check
			out = MaterialSampleOutput::Reject(MST_SpecularReflection);
			return;
		}

		const float jacobian = 1 / (4 * HdotV * HdotV);

		// Evaluate D*G*(V.H*L.H)/(V.N) but ignore pdf
		float _pdf;
		const float gd = evalGD(H, in.Context.V, out.L, in.ShadingContext, _pdf);

		out.Weight = fresnel * mSpecularity->eval(in.ShadingContext) * (gd * jacobian);
		out.Type   = MST_SpecularReflection;
		out.PDF_S  = pdf * jacobian;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <RoughConductorMaterial>:" << std::endl
			   << "    Eta:         " << mEta->dumpInformation() << std::endl
			   << "    K:           " << mK->dumpInformation() << std::endl
			   << "    Specularity: " << mSpecularity->dumpInformation() << std::endl
			   << "    RoughnessX:  " << mRoughnessX->dumpInformation() << std::endl
			   << "    RoughnessY:  " << mRoughnessY->dumpInformation() << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mEta;
	const std::shared_ptr<FloatSpectralNode> mK;
	const std::shared_ptr<FloatSpectralNode> mSpecularity;
	const std::shared_ptr<FloatScalarNode> mRoughnessX;
	const std::shared_ptr<FloatScalarNode> mRoughnessY;
};

class RoughConductorMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		std::shared_ptr<FloatScalarNode> rx;
		std::shared_ptr<FloatScalarNode> ry;

		if (ctx.parameters().hasParameter("roughness_x"))
			rx = ctx.lookupScalarNode("roughness_x", 0);
		else
			rx = ctx.lookupScalarNode("roughness", 0);

		if (ctx.parameters().hasParameter("roughness_y"))
			ry = ctx.lookupScalarNode("roughness_y", 0);
		else
			ry = rx;

		const auto eta	= ctx.lookupSpectralNode("eta", 1.2f);
		const auto k	= ctx.lookupSpectralNode("k", 2.605f);
		const auto spec = ctx.lookupSpectralNode("specularity", 1);

		if (rx == ry)
			return std::make_shared<RoughConductorMaterial<false>>(eta, k, spec, rx, ry);
		else
			return std::make_shared<RoughConductorMaterial<true>>(eta, k, spec, rx, ry);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "roughconductor", "roughmetal" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::RoughConductorMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)