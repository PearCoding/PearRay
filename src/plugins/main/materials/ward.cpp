#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Microfacet.h"
#include "math/Sampling.h"
#include "math/Scattering.h"
#include "math/Spherical.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

class WardMaterial : public IMaterial {
public:
	WardMaterial(const std::shared_ptr<FloatSpectralNode>& alb,
				 const std::shared_ptr<FloatSpectralNode>& spec,
				 const std::shared_ptr<FloatScalarNode>& reflectivity,
				 const std::shared_ptr<FloatScalarNode>& roughnessX,
				 const std::shared_ptr<FloatScalarNode>& roughnessY)
		: IMaterial()
		, mAlbedo(alb)
		, mSpecularity(spec)
		, mReflectivity(reflectivity)
		, mRoughnessX(roughnessX)
		, mRoughnessY(roughnessY)
	{
	}

	virtual ~WardMaterial() = default;

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto& sctx = in.ShadingContext;
		out.Type		 = MaterialScatteringType::DiffuseReflection;

		// Ward is only defined in the positive hemisphere
		if (in.Context.NdotV() <= PR_EPSILON || in.Context.NdotL() <= PR_EPSILON) {
			out.Weight = 0.0f;
			out.PDF_S  = 0.0f;
			return;
		}

		ShadingVector H = Scattering::faceforward(Scattering::halfway_reflection(in.Context.V, in.Context.L));

		float spec;
		if (mRoughnessX == mRoughnessY) {
			spec = std::min(1.0f,
							Microfacet::ward(mRoughnessX->eval(sctx),
											 in.Context.NdotV(), in.Context.NdotL(), H.cosTheta()));
		} else {
			spec = std::min(1.0f,
							Microfacet::ward(mRoughnessX->eval(sctx),
											 mRoughnessY->eval(sctx),
											 in.Context.NdotV(), in.Context.NdotL(),
											 H.cosTheta(), H.sinPhi(), H.cosPhi()));
		}

		const float refl = mReflectivity->eval(sctx);
		out.Weight		 = mAlbedo->eval(sctx) * (1 - refl) + spec * mSpecularity->eval(sctx) * refl;
		out.Weight *= std::max(0.0f, in.Context.NdotL());

		out.PDF_S = std::min(std::max(Sampling::cos_hemi_pdf(in.Context.NdotL()) * (1 - refl) + spec * refl, 0.0f), 1.0f);
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		// Ward is only defined in the positive hemisphere
		if (in.Context.NdotV() <= PR_EPSILON || in.Context.NdotL() <= PR_EPSILON) {
			out.PDF_S = 0.0f;
			return;
		}

		ShadingVector H = Scattering::faceforward(Scattering::halfway_reflection(in.Context.V, in.Context.L));

		float spec;
		if (mRoughnessX == mRoughnessY) {
			spec = std::min(1.0f,
							Microfacet::ward(mRoughnessX->eval(in.ShadingContext),
											 in.Context.NdotV(), in.Context.NdotL(), H.cosTheta()));
		} else {
			spec = std::min(1.0f,
							Microfacet::ward(mRoughnessX->eval(in.ShadingContext),
											 mRoughnessY->eval(in.ShadingContext),
											 in.Context.NdotV(), in.Context.NdotL(),
											 H.cosTheta(), H.sinPhi(), H.cosPhi()));
		}

		const float refl = mReflectivity->eval(in.ShadingContext);
		out.PDF_S		 = std::min(std::max(Sampling::cos_hemi_pdf(in.Context.NdotL()) * (1 - refl) + spec * refl, 0.0f), 1.0f);
	}

	void sampleDiffusePath(const MaterialSampleInput& in, const ShadingContext& sctx, MaterialSampleOutput& out) const
	{
		out.L			   = Sampling::cos_hemi(in.RND.getFloat(), in.RND.getFloat());
		out.PDF_S		   = Sampling::cos_hemi_pdf(out.L(2));
		out.IntegralWeight = mAlbedo->eval(sctx);
		out.Type		   = MaterialScatteringType::DiffuseReflection;
	}

	void sampleSpecularPath(const MaterialSampleInput& in, const ShadingContext& sctx, MaterialSampleOutput& out) const
	{
		const float m1 = mRoughnessX->eval(sctx);
		const float m2 = mRoughnessY->eval(sctx);
		const float u0 = in.RND.getFloat();
		const float u1 = in.RND.getFloat();

		float cosTheta, sinTheta;			 // V samples
		float cosPhi, sinPhi;				 // U samples
		if (std::abs(m1 - m2) <= PR_EPSILON) // Isotropic
		{
			sinPhi = std::sin(2 * PR_PI * u0);
			cosPhi = std::cos(2 * PR_PI * u0);

			const float f = -std::log(std::max(0.00001f, u1)) * m1 * m1;
			cosTheta	  = 1 / (1 + f);
			sinTheta	  = std::sqrt(f) * cosTheta;

			const float t = 4 * PR_PI * m1 * m1 * cosTheta * cosTheta * cosTheta * u1;
			if (t <= PR_EPSILON)
				out.PDF_S = 1;
			else
				out.PDF_S = 1 / t;
		} else {
			const float pm1 = m1 * m1;
			const float pm2 = m2 * m2;

			const float f1 = (m2 / m1) * std::tan(2 * PR_PI * u0);
			cosPhi		   = 1 / std::sqrt(1 + f1 * f1);
			sinPhi		   = f1 * cosPhi;

			const float cosPhi2 = cosPhi * cosPhi;
			const float tz		= (cosPhi2 / pm1 + sinPhi * sinPhi / pm2);
			const float f2		= -std::log(std::max(0.000001f, u1)) / tz;
			cosTheta			= 1 / (1 + f2);
			sinTheta			= std::sqrt(f2) * cosTheta;

			const float cosTheta2 = cosTheta * cosTheta;
			const float tu		  = pm1 * sinPhi * sinPhi + pm2 * cosPhi2;
			const float tb		  = 4 * PR_PI * m1 * m2 * (pm1 * (1 - cosPhi2) / cosPhi + pm2 * cosPhi) * cosTheta2;
			out.PDF_S			  = tu / tb * std::exp(-tz * (1 - cosTheta2) / (cosTheta2));
		}

		Vector3f H		   = Spherical::cartesian(sinTheta, cosTheta, sinPhi, cosPhi);
		out.L			   = Scattering::reflect(in.Context.V, H);
		out.Type		   = MaterialScatteringType::SpecularReflection;
		out.IntegralWeight = mSpecularity->eval(sctx) * std::max(0.0f, out.L[2]);
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto& sctx = in.ShadingContext;
		const float refl = mReflectivity->eval(sctx);

		if (in.RND.getFloat() <= refl) {
			sampleSpecularPath(in, sctx, out);
			out.PDF_S /= refl;
		} else {
			sampleDiffusePath(in, sctx, out);
			out.PDF_S /= 1.0f - refl;
		}
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <WardMaterial>:" << std::endl
			   << "    Albedo:       " << (mAlbedo ? mAlbedo->dumpInformation() : "NONE") << std::endl
			   << "    Specularity:  " << (mSpecularity ? mSpecularity->dumpInformation() : "NONE") << std::endl
			   << "    Reflectivity: " << (mReflectivity ? mReflectivity->dumpInformation() : "NONE") << std::endl
			   << "    RoughnessX:   " << (mRoughnessX ? mRoughnessX->dumpInformation() : "NONE") << std::endl
			   << "    RoughnessY:   " << (mRoughnessY ? mRoughnessY->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralNode> mAlbedo;
	std::shared_ptr<FloatSpectralNode> mSpecularity;
	std::shared_ptr<FloatScalarNode> mReflectivity;
	std::shared_ptr<FloatScalarNode> mRoughnessX;
	std::shared_ptr<FloatScalarNode> mRoughnessY;
};

class WardMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		std::shared_ptr<FloatScalarNode> roughnessX;
		std::shared_ptr<FloatScalarNode> roughnessY;
		const auto roughnessP = params.getParameter("roughness");

		if (roughnessP.isValid()) {
			roughnessX = ctx.lookupScalarNode(roughnessP, 0.5f);
			roughnessY = roughnessX;
		} else {
			roughnessX = ctx.lookupScalarNode("roughness_x", 0.5f);
			roughnessY = ctx.lookupScalarNode("roughness_y", 0.5f);
		}

		return std::make_shared<WardMaterial>(ctx.lookupSpectralNode("albedo", 1),
											  ctx.lookupSpectralNode("specularity", 1),
											  ctx.lookupScalarNode("reflectivity", 0.5f),
											  roughnessX, roughnessY);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "ward" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("OrenNayar BSDF", "A not so perfectly diffuse BSDF")
			.Identifiers(getNames())
			.Inputs()
			.SpectralNodeV({ "albedo", "base", "diffuse" }, "Amount of light which is reflected", 1.0f)
			.SpectralNode("specularity", "Tint", 1.0f)
			.ScalarNode("reflectivity", "Amount of reflection", 0.5f)
			.BeginBlock("Roughness", PluginParamDescBlockOp::OneOf)
			.ScalarNode("roughness", "Isotropic roughness", 0.0f)
			.BeginBlock("")
			.ScalarNode("roughness_x", "Anisotropic x roughness", 0.0f)
			.ScalarNode("roughness_y", "Anisotropic y roughness", 0.0f)
			.EndBlock()
			.EndBlock()
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::WardMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)