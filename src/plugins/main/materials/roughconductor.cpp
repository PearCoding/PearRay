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

#include "Roughness.h"

#include <sstream>

namespace PR {
template <bool UseVNDF, bool IsAnisotropic>
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
		, mRoughness(roughnessX, roughnessY)
	{
	}

	virtual ~RoughConductorMaterial() = default;

	inline SpectralBlob fresnelTerm(float dot, const ShadingContext& sctx) const
	{
		const SpectralBlob eta = mEta->eval(sctx);
		const SpectralBlob k   = mK->eval(sctx);

		SpectralBlob fresnel;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			fresnel[i] = Fresnel::conductor(dot, eta[i], k[i]);

		return fresnel;
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

		const SpectralBlob fresnel = fresnelTerm(in.Context.V.absCosTheta(), in.ShadingContext);

		float pdf;
		const float gd		 = mRoughness.eval(H, in.Context.V, in.Context.L, in.ShadingContext, pdf);
		const float jacobian = 1 / (4 * HdotV * HdotV);
		out.Weight			 = fresnel * mSpecularity->eval(in.ShadingContext) * (gd * jacobian);
		out.PDF_S			 = pdf * jacobian;
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

		const float jacobian = 1 / (4 * HdotV * HdotV);
		const float pdf		 = mRoughness.pdf(H, in.Context.V, in.ShadingContext);
		out.PDF_S			 = pdf * jacobian;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		// Sample microfacet normal
		float pdf;
		bool delta;
		const Vector3f H = mRoughness.sample(in.RND, in.Context.V, in.ShadingContext, pdf, delta);

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

		const SpectralBlob fresnel = fresnelTerm(in.Context.V.absCosTheta(), in.ShadingContext);
		out.Weight				   = fresnel * mSpecularity->eval(in.ShadingContext);
		out.Type				   = MST_SpecularReflection;
		out.PDF_S				   = pdf;

		if (delta) {
			out.Flags = MSF_DeltaDistribution;
		} else {
			// Evaluate D*G*(V.H*L.H)/(V.N) but ignore pdf
			float _pdf;
			const float gd		 = mRoughness.eval(H, in.Context.V, out.L, in.ShadingContext, _pdf);
			const float jacobian = 1 / (4 * HdotV * HdotV);
			out.Weight *= gd * jacobian;
			out.PDF_S *= jacobian;
		}
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <RoughConductorMaterial>:" << std::endl
			   << "    Eta:         " << mEta->dumpInformation() << std::endl
			   << "    K:           " << mK->dumpInformation() << std::endl
			   << "    Specularity: " << mSpecularity->dumpInformation() << std::endl
			   << "    RoughnessX:  " << mRoughness.roughnessX()->dumpInformation() << std::endl
			   << "    RoughnessY:  " << mRoughness.roughnessY()->dumpInformation() << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mEta;
	const std::shared_ptr<FloatSpectralNode> mK;
	const std::shared_ptr<FloatSpectralNode> mSpecularity;
	const Roughness<IsAnisotropic, UseVNDF, false> mRoughness;
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

		const auto eta	= ctx.lookupSpectralNode({ "eta", "index", "ior" }, 1.2f);
		const auto k	= ctx.lookupSpectralNode({ "k", "kappa" }, 2.605f);
		const auto spec = ctx.lookupSpectralNode("specularity", 1);

		// TODO: Fix VNDF and make it default again
		const bool use_vndf = ctx.parameters().getBool("vndf", false);

		if (use_vndf) {
			if (rx == ry)
				return std::make_shared<RoughConductorMaterial<true, false>>(eta, k, spec, rx, ry);
			else
				return std::make_shared<RoughConductorMaterial<true, true>>(eta, k, spec, rx, ry);
		} else {
			if (rx == ry)
				return std::make_shared<RoughConductorMaterial<false, false>>(eta, k, spec, rx, ry);
			else
				return std::make_shared<RoughConductorMaterial<false, true>>(eta, k, spec, rx, ry);
		}
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