#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Tangent.h"
#include "renderer/RenderContext.h"
#include "spectral/OrderedSpectrum.h"

#include <sstream>

// This is nasty and only a workaround to fix a ISO C++ pedantic warning which is traited as error in -Werror
#ifndef _MSC_VER
#define _MSC_VER
#define ICHANGEDIT
#endif

#define POWITACQ_IMPLEMENTATION 1
#include "powitacq.h"

#ifdef ICHANGEDIT
#undef _MSC_VER
#endif

namespace PR {

class RGLMeasuredMaterial : public IMaterial {
public:
	RGLMeasuredMaterial(uint32 id, const std::string& filename, const std::shared_ptr<FloatSpectralNode>& tint)
		: IMaterial(id)
		, mFilename(filename)
		, mBRDF(filename.c_str())
		, mTint(tint)
	{
	}

	virtual ~RGLMeasuredMaterial() = default;

	void startGroup(size_t, const RenderTileSession&) override
	{
	}

	void endGroup() override
	{
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		powitacq::Vector3f wi = powitacq::Vector3f(in.Context.V(0), in.Context.V(1), in.Context.V(2));
		powitacq::Vector3f wo = powitacq::Vector3f(in.Context.L(0), in.Context.L(1), in.Context.L(2));

		out.Weight = mTint->eval(in.ShadingContext) * eval(wi, wo, in.Context.WavelengthNM);
		out.PDF_S  = mBRDF.pdf(wi, wo);
		out.Type   = MST_DiffuseReflection;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		powitacq::Vector2f u  = powitacq::Vector2f(in.RND(0), in.RND(1));
		powitacq::Vector3f wi = powitacq::Vector3f(in.Context.V(0), in.Context.V(1), in.Context.V(2));
		powitacq::Vector3f wo;
		float pdf;
		mBRDF.sample(u, wi, &wo, &pdf);

		out.Weight = mTint->eval(in.ShadingContext) * eval(wi, wo, in.Context.WavelengthNM);
		out.Type   = MST_DiffuseReflection;
		out.L	   = Vector3f(wo[0], wo[1], wo[2]);
		out.PDF_S  = pdf;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <RGLMeasuredMaterial>:" << std::endl
			   << "    Filename: " << mFilename << std::endl
			   << "    Tint:     " << mTint << std::endl;

		return stream.str();
	}

private:
	inline SpectralBlob eval(const powitacq::Vector3f& wi, const powitacq::Vector3f& wo, const SpectralBlob& wvls) const
	{
		SpectralBlob out;
		auto spec = mBRDF.eval(wi, wo);

		PR_ASSERT(spec.size() == mBRDF.wavelengths().size(), "Expected spectrum information to be equally sized");
		OrderedSpectrumView view(&spec[0], &mBRDF.wavelengths()[0], spec.size());

		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out[i] = view.lookup(wvls[i]);

		return out;
	}

	const std::string mFilename;
	powitacq::BRDF mBRDF;
	const std::shared_ptr<FloatSpectralNode> mTint;
};

class RGLMeasuredMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;
		return std::make_shared<RGLMeasuredMaterial>(id,
													 ctx.escapePath(params.getString("filename", "")),
													 ctx.Env->lookupSpectralNode(params.getParameter("tint"), 1));
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "rgl", "rgl_measured", "rgl-measured" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::RGLMeasuredMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)