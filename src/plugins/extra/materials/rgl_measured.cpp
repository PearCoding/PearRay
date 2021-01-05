#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Tangent.h"
#include "renderer/RenderContext.h"
#include "spectral/OrderedSpectrum.h"

#include "Logger.h"

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
	RGLMeasuredMaterial(const std::filesystem::path& filename, const std::shared_ptr<FloatSpectralNode>& tint)
		: IMaterial()
		, mFilename(filename)
		, mBRDF(filename.generic_string())
		, mTint(tint)
	{
	}

	virtual ~RGLMeasuredMaterial() = default;

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		powitacq::Vector3f wi = powitacq::Vector3f(in.Context.V(0), in.Context.V(1), in.Context.V(2));
		powitacq::Vector3f wo = powitacq::Vector3f(in.Context.L(0), in.Context.L(1), in.Context.L(2));

		out.Weight = mTint->eval(in.ShadingContext) * eval(wi, wo, in.Context.WavelengthNM);
		out.PDF_S  = mBRDF.pdf(wi, wo);
		out.Type   = MaterialScatteringType::DiffuseReflection;
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		powitacq::Vector3f wi = powitacq::Vector3f(in.Context.V(0), in.Context.V(1), in.Context.V(2));
		powitacq::Vector3f wo = powitacq::Vector3f(in.Context.L(0), in.Context.L(1), in.Context.L(2));

		out.PDF_S = mBRDF.pdf(wi, wo);
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		powitacq::Vector2f u  = powitacq::Vector2f(in.RND.getFloat(), in.RND.getFloat());
		powitacq::Vector3f wi = powitacq::Vector3f(in.Context.V(0), in.Context.V(1), in.Context.V(2));
		powitacq::Vector3f wo;
		float pdf;
		mBRDF.sample(u, wi, &wo, &pdf);

		out.Weight = mTint->eval(in.ShadingContext) * eval(wi, wo, in.Context.WavelengthNM);
		out.Type   = MaterialScatteringType::DiffuseReflection;
		out.L	   = Vector3f(wo[0], wo[1], wo[2]);
		out.PDF_S  = pdf;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <RGLMeasuredMaterial>:" << std::endl
			   << "    Filename: " << mFilename << std::endl
			   << "    Tint:     " << mTint->dumpInformation() << std::endl;

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

	const std::filesystem::path mFilename;
	powitacq::BRDF mBRDF;
	const std::shared_ptr<FloatSpectralNode> mTint;
};

class RGLMeasuredMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.parameters();
		const auto path				 = ctx.escapePath(params.getString("filename", ""));
		if (path.empty()) {
			PR_LOG(L_ERROR) << "Could not create measured material as filename was empty" << std::endl;
			return nullptr;
		}

		return std::make_shared<RGLMeasuredMaterial>(path, ctx.lookupSpectralNode("tint", 1));
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "rgl", "rgl_measured", "rgl-measured" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("RGL BSDF", "RGL measured BSDF")
			.Identifiers(getNames())
			.Inputs()
			.Filename("filename", "A path to a file from the RGL database")
			.SpectralNode("tint", "Tint", true)
			.Specification()
			.get();
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::RGLMeasuredMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)