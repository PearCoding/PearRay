#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Projection.h"
#include "math/Scattering.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

template <bool SpectralVarying>
class ConductorMaterial : public IMaterial {
public:
	ConductorMaterial(const std::shared_ptr<FloatSpectralNode>& eta, const std::shared_ptr<FloatSpectralNode>& k,
					  const std::shared_ptr<FloatSpectralNode>& spec)
		: IMaterial()
		, mEta(eta)
		, mK(k)
		, mSpecularity(spec)
	{
	}

	virtual ~ConductorMaterial() = default;

	int flags() const override { return MF_OnlyDeltaDistribution; }

	void eval(const MaterialEvalInput&, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		PR_ASSERT(false, "Delta distribution materials should not be evaluated");

		out.PDF_S  = 0.0f;
		out.Type   = MST_SpecularReflection;
		out.Weight = SpectralBlob::Zero();
	}

	void pdf(const MaterialEvalInput&, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		PR_ASSERT(false, "Delta distribution materials should not be evaluated");

		out.PDF_S = 0;
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
			fresnel[i] = Fresnel::conductor(in.Context.V.absCosTheta(), 1, eta[i], k[i]);

		out.Weight = fresnel * mSpecularity->eval(in.ShadingContext);
		out.Type   = MST_SpecularReflection;
		out.PDF_S  = 1;
		out.L	   = Scattering::reflect(in.Context.V);

		if constexpr (SpectralVarying)
			out.Flags = MSF_DeltaDistribution | MSF_SpectralVarying;
		else
			out.Flags = MSF_DeltaDistribution;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <ConductorMaterial>:" << std::endl
			   << "    Eta:             " << mEta->dumpInformation() << std::endl
			   << "    K:               " << mK->dumpInformation() << std::endl
			   << "    Specularity:     " << mSpecularity->dumpInformation() << std::endl
			   << "    SpectralVarying: " << (SpectralVarying ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mEta;
	const std::shared_ptr<FloatSpectralNode> mK;
	const std::shared_ptr<FloatSpectralNode> mSpecularity;
};

class ConductorMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		// Construct rough conductor instead
		if (ctx.parameters().hasParameter("roughness")
			|| ctx.parameters().hasParameter("roughness_x")
			|| ctx.parameters().hasParameter("roughness_y"))
			return ctx.loadMaterial("roughconductor", ctx.parameters());

		const auto eta	= ctx.lookupSpectralNode({ "eta", "index", "ior" }, 1.2f);
		const auto k	= ctx.lookupSpectralNode({ "k", "kappa" }, 2.605f);
		const auto spec = ctx.lookupSpectralNode("specularity", 1);

		// Contrary to the dielectric interface, conductors are not spectral varying per default
		// as it is quite uncommon to use ior functions
		const bool spectralVarying = ctx.parameters().getBool("spectral_varying", false);
		if (spectralVarying)
			return std::make_shared<ConductorMaterial<true>>(eta, k, spec);
		else
			return std::make_shared<ConductorMaterial<false>>(eta, k, spec);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "conductor", "metal" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::ConductorMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)