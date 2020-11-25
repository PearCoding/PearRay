#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Sampling.h"
#include "math/Tangent.h"
#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

template <bool TwoSided>
class LambertMaterial : public IMaterial {
public:
	LambertMaterial(uint32 id, const std::shared_ptr<FloatSpectralNode>& alb)
		: IMaterial(id)
		, mAlbedo(alb)
	{
	}

	virtual ~LambertMaterial() = default;

	inline static float culling(float u)
	{
		if constexpr (TwoSided)
			return std::abs(u);
		else
			return std::max(0.0f, u);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const float dot = in.Context.V.sameHemisphere(in.Context.L) ? culling(in.Context.NdotL()) : 0;
		out.Weight		= mAlbedo->eval(in.ShadingContext) * dot * PR_INV_PI;
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
		out.Type		= MST_DiffuseReflection;
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const float dot = in.Context.V.sameHemisphere(in.Context.L) ? culling(in.Context.NdotL()) : 0;
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		out.L = Sampling::cos_hemi(in.RND[0], in.RND[1]);

		out.Weight = mAlbedo->eval(in.ShadingContext) * out.L(2) * PR_INV_PI;
		out.PDF_S  = Sampling::cos_hemi_pdf(out.L(2));
		out.Type   = MST_DiffuseReflection;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <DiffuseMaterial>:" << std::endl
			   << "    Albedo:   " << mAlbedo->dumpInformation() << std::endl
			   << "    TwoSided: " << (TwoSided ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mAlbedo;
};

class LambertMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		if (ctx.parameters().getBool("two_sided", true))
			return std::make_shared<LambertMaterial<true>>(id, ctx.lookupSpectralNode("albedo", 1));
		else
			return std::make_shared<LambertMaterial<false>>(id, ctx.lookupSpectralNode("albedo", 1));
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "diffuse", "lambert" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::LambertMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)