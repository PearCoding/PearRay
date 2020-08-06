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

class LambertMaterial : public IMaterial {
public:
	LambertMaterial(uint32 id, const std::shared_ptr<FloatSpectralNode>& alb)
		: IMaterial(id)
		, mAlbedo(alb)
	{
	}

	virtual ~LambertMaterial() = default;

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const float dot = std::max(0.0f, in.Context.NdotL());
		out.Weight		= mAlbedo->eval(in.ShadingContext) * dot * PR_INV_PI;
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
		out.Type		= MST_DiffuseReflection;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		float pdf;
		out.L = Sampling::cos_hemi(in.RND[0], in.RND[1], pdf);

		out.Weight = mAlbedo->eval(in.ShadingContext) * out.L(2) * PR_INV_PI;
		out.Type   = MST_DiffuseReflection;
		out.PDF_S  = pdf;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <DiffuseMaterial>:" << std::endl
			   << "    Albedo: " << (mAlbedo ? mAlbedo->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralNode> mAlbedo;
};

class LambertMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;
		return std::make_shared<LambertMaterial>(id, ctx.Env->lookupSpectralNode(params.getParameter("albedo"), 1));
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