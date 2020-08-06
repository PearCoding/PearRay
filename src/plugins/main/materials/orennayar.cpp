#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Sampling.h"
#include "math/Tangent.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

class OrenNayarMaterial : public IMaterial {
public:
	OrenNayarMaterial(uint32 id,
					  const std::shared_ptr<FloatSpectralNode>& alb,
					  const std::shared_ptr<FloatScalarNode>& rough)
		: IMaterial(id)
		, mAlbedo(alb)
		, mRoughness(rough)
	{
	}

	virtual ~OrenNayarMaterial() = default;

	// https://mimosa-pudica.net/improved-oren-nayar.html
	inline SpectralBlob calc(const Vector3f& L, float NdotL, const MaterialSampleContext& ctx, const ShadingContext& sctx) const
	{
		float roughness = mRoughness->eval(sctx);
		roughness *= roughness;

		SpectralBlob weight = mAlbedo->eval(sctx);

		if (roughness > PR_EPSILON) {
			const float s = -NdotL * ctx.NdotV() + ctx.V.dot(L);
			const float t = s < PR_EPSILON ? 1.0f : std::max(NdotL, ctx.NdotV());

			const SpectralBlob A = SpectralBlob::Ones() * (1 - 0.5f * roughness / (roughness + 0.33f))
								   + 0.17f * weight * roughness / (roughness + 0.13f);
			const float B = 0.45f * roughness / (roughness + 0.09f);
			weight *= A + SpectralBlob::Ones() * (B * s / t);
		}

		return weight * PR_INV_PI * std::abs(NdotL);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const float dot = std::max(0.0f, in.Context.NdotL());
		out.Weight		= calc(in.Context.L, dot, in.Context, in.ShadingContext);
		out.Type		= MST_DiffuseReflection;
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		float pdf;
		out.L = Sampling::cos_hemi(in.RND[0], in.RND[1], pdf);

		float NdotL = std::max(0.0f, out.L(2));
		out.Weight	= calc(out.L, NdotL, in.Context, in.ShadingContext);
		out.Type	= MST_DiffuseReflection;
		out.PDF_S	= pdf;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <OrenNayarMaterial>:" << std::endl
			   << "    Albedo: " << (mAlbedo ? mAlbedo->dumpInformation() : "NONE") << std::endl
			   << "    Roughness: " << (mRoughness ? mRoughness->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralNode> mAlbedo;
	std::shared_ptr<FloatScalarNode> mRoughness;
};

class OrenNayarMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;
		return std::make_shared<OrenNayarMaterial>(id,
												   ctx.Env->lookupSpectralNode(params.getParameter("albedo"), 1),
												   ctx.Env->lookupScalarNode(params.getParameter("roughness"), 0.5f));
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "orennayar", "oren", "rough" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::OrenNayarMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)