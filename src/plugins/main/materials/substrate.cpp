#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"

#include "Roughness.h"

#include <sstream>

namespace PR {

/// Pbrt like substrate material
/// Sampling is purely based on microfacets, not the fresnel term
template <bool UseVNDF, bool IsAnisotropic>
class SubstrateMaterial : public IMaterial {
public:
	explicit SubstrateMaterial(const std::shared_ptr<FloatSpectralNode>& ior,
							   const std::shared_ptr<FloatSpectralNode>& alb, const std::shared_ptr<FloatSpectralNode>& spec,
							   const std::shared_ptr<FloatScalarNode>& roughnessX,
							   const std::shared_ptr<FloatScalarNode>& roughnessY)
		: IMaterial()
		, mIOR(ior)
		, mAlbedo(alb)
		, mSpecularity(spec)
		, mRoughness(roughnessX, roughnessY)
	{
	}

	virtual ~SubstrateMaterial() = default;

	inline SpectralBlob fresnelTerm(float dot, const ShadingContext& sctx) const
	{
		const SpectralBlob n1 = SpectralBlob::Ones();
		const SpectralBlob n2 = mIOR->eval(sctx);

		SpectralBlob res;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			res[i] = Fresnel::dielectric(dot, n1[i], n2[i]);
		return res;
	}

	inline float fresnelTermHero(float dot, const ShadingContext& sctx) const
	{
		const SpectralBlob n1 = SpectralBlob::Ones();
		const SpectralBlob n2 = mIOR->eval(sctx);

		return Fresnel::dielectric(dot, n1[0], n2[0]);
	}

	inline SpectralBlob evalDiff(const ShadingContext& sctx) const
	{
		return mAlbedo->eval(sctx) * PR_INV_PI;
	}

	inline SpectralBlob evalSpec(const SpectralBlob& fresnel, const ShadingContext& sctx) const
	{
		return fresnel * mSpecularity->eval(sctx);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		out.Type = MST_SpecularReflection;
		if (!in.Context.V.sameHemisphere(in.Context.L)) {
			out.Weight = 0;
			out.PDF_S  = 0;
			return;
		}

		const ShadingVector H = Scattering::halfway_reflection(in.Context.V, in.Context.L);
		const float HdotV	  = H.dot(in.Context.V);
		PR_ASSERT(HdotV >= 0.0f, "HdotV must be positive");

		float pdf;
		const float gd			   = mRoughness.eval(H, in.Context.V, in.Context.L, in.ShadingContext, pdf);
		const float jacobian	   = 1 / (4 * HdotV * HdotV);
		const SpectralBlob fresnel = fresnelTerm(HdotV, in.ShadingContext);

		out.Weight = evalDiff(in.ShadingContext) * in.Context.L.absCosTheta() + evalSpec(fresnel, in.ShadingContext) * (gd * jacobian);
		out.PDF_S  = pdf * jacobian;

		// Specular + Albedo might be greater than 1, clamp it
		out.Weight = out.Weight.cwiseMin(1.0f);
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
		const SpectralBlob fresnel = fresnelTerm(HdotV, in.ShadingContext);
		out.L					   = Scattering::reflect(in.Context.V, H);

		if (!in.Context.V.sameHemisphere(out.L)) { // Side check
			out = MaterialSampleOutput::Reject(MST_SpecularReflection);
			return;
		}

		out.Type  = MST_SpecularReflection;
		out.PDF_S = pdf;

		if (delta) {
			out.Weight = evalSpec(fresnel, in.ShadingContext) * out.L.absCosTheta();
			out.Flags  = MSF_DeltaDistribution;
		} else {
			// Evaluate D*G*(V.H*L.H)/(V.N) but ignore pdf
			float _pdf;
			const float gd		 = mRoughness.eval(H, in.Context.V, out.L, in.ShadingContext, _pdf);
			const float jacobian = 1 / (4 * HdotV * HdotV);
			out.Weight			 = evalSpec(fresnel, in.ShadingContext) * (gd * jacobian);
			out.PDF_S *= jacobian;
		}

		// Add diffuse term
		out.Weight += evalDiff(in.ShadingContext) * out.L.absCosTheta();

		// Specular + Albedo might be greater than 1, clamp it
		out.Weight = out.Weight.cwiseMin(1.0f);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << IMaterial::dumpInformation()
			   << "  <SubstrateMaterial>:" << std::endl
			   << "    IOR:         " << mIOR->dumpInformation() << std::endl
			   << "    Albedo:      " << mAlbedo->dumpInformation() << std::endl
			   << "    Specularity: " << mSpecularity->dumpInformation() << std::endl
			   << "    RoughnessX:  " << mRoughness.roughnessX()->dumpInformation() << std::endl
			   << "    RoughnessY:  " << mRoughness.roughnessY()->dumpInformation() << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mIOR;
	const std::shared_ptr<FloatSpectralNode> mAlbedo;
	const std::shared_ptr<FloatSpectralNode> mSpecularity;

	const Roughness<IsAnisotropic, UseVNDF, false> mRoughness;
};

class SubstrateMaterialPlugin : public IMaterialPlugin {
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

		const auto ior	= ctx.lookupSpectralNode("index", 1.55f);
		const auto alb	= ctx.lookupSpectralNode("albedo", 0.5f);
		const auto spec = ctx.lookupSpectralNode("specularity", 1);

		// TODO: Fix VNDF and make it default again
		const bool use_vndf = ctx.parameters().getBool("vndf", false);

		if (use_vndf) {
			if (rx == ry)
				return std::make_shared<SubstrateMaterial<true, false>>(ior, alb, spec, rx, ry);
			else
				return std::make_shared<SubstrateMaterial<true, true>>(ior, alb, spec, rx, ry);
		} else {
			if (rx == ry)
				return std::make_shared<SubstrateMaterial<false, false>>(ior, alb, spec, rx, ry);
			else
				return std::make_shared<SubstrateMaterial<false, true>>(ior, alb, spec, rx, ry);
		}
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "substrate", "plastic", "roughplastic", "coateddiffuse" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SubstrateMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)