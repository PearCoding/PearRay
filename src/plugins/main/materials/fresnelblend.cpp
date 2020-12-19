#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"

#include <sstream>

namespace PR {

enum MaterialDelta {
	MD_None	  = 0,
	MD_First  = 1,
	MD_Second = 2,
	MD_All
};

template <MaterialDelta Delta, bool SpectralVarying>
class FresnelBlendMaterial : public IMaterial {
public:
	explicit FresnelBlendMaterial(const std::shared_ptr<IMaterial>& material0, const std::shared_ptr<IMaterial>& material1,
								  const std::shared_ptr<FloatSpectralNode>& ior)
		: IMaterial()
		, mMaterials{ material0, material1 }
		, mIOR(ior)
	{
	}

	virtual ~FresnelBlendMaterial() = default;

	int flags() const override
	{
		if constexpr (Delta == MD_All)
			return MF_OnlyDeltaDistribution;
		else
			return 0;
	}

	inline SpectralBlob fresnelTerm(float dot, const ShadingContext& sctx) const
	{
		SpectralBlob n1 = SpectralBlob::Ones();
		SpectralBlob n2 = mIOR->eval(sctx);

		if (dot < 0)
			std::swap(n1, n2);

		SpectralBlob res;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			res[i] = Fresnel::dielectric(std::abs(dot), n1[i], n2[i]);
		return res;
	}

	inline float fresnelTermHero(float dot, const ShadingContext& sctx) const
	{
		SpectralBlob n1 = SpectralBlob::Ones();
		SpectralBlob n2 = mIOR->eval(sctx);

		if (dot < 0)
			std::swap(n1, n2);

		return Fresnel::dielectric(std::abs(dot), n1[0], n2[0]);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession& session) const override
	{
		PR_PROFILE_THIS;

		if constexpr (Delta == MD_All) {
			PR_ASSERT(false, "Delta distribution materials should not be evaluated");
			out.PDF_S  = 0.0f;
			out.Type   = MST_SpecularTransmission;
			out.Weight = SpectralBlob::Zero();
		} else if constexpr (Delta == MD_First) {
			mMaterials[1]->eval(in, out, session);

			const SpectralBlob fresnel = fresnelTerm(in.Context.NdotV(), in.ShadingContext);
			out.PDF_S *= fresnel[0]; // Hero only
			out.Weight *= fresnel;
		} else if constexpr (Delta == MD_Second) {
			mMaterials[0]->eval(in, out, session);

			const SpectralBlob fresnel = fresnelTerm(in.Context.NdotV(), in.ShadingContext);
			out.PDF_S *= (1 - fresnel[0]);
			out.Weight *= (1 - fresnel);
		} else {
			const SpectralBlob fresnel = fresnelTerm(in.Context.NdotV(), in.ShadingContext);

			MaterialEvalOutput out1;
			mMaterials[0]->eval(in, out1, session);
			MaterialEvalOutput out2;
			mMaterials[1]->eval(in, out2, session);

			out.PDF_S  = (1 - fresnel[0]) * out1.PDF_S + fresnel[0] * out2.PDF_S;
			out.Weight = (1 - fresnel) * out1.Weight + fresnel * out2.Weight;
			out.Type   = fresnel[0] <= 0.5f ? out1.Type : out2.Type; // TODO
		}
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession& session) const override
	{
		PR_PROFILE_THIS;
		if constexpr (Delta == MD_All) {
			PR_ASSERT(false, "Delta distribution materials should not be evaluated");
			out.PDF_S = 0.0f;
		} else if constexpr (Delta == MD_First) {
			mMaterials[1]->pdf(in, out, session);
			const float prob = fresnelTermHero(in.Context.NdotV(), in.ShadingContext);
			out.PDF_S *= prob;
		} else if constexpr (Delta == MD_Second) {
			mMaterials[0]->pdf(in, out, session);
			const float prob = fresnelTermHero(in.Context.NdotV(), in.ShadingContext);
			out.PDF_S *= (1 - prob);
		} else {
			const float prob = fresnelTermHero(in.Context.NdotV(), in.ShadingContext);

			MaterialPDFOutput out1;
			mMaterials[0]->pdf(in, out1, session);
			MaterialPDFOutput out2;
			mMaterials[1]->pdf(in, out2, session);

			out.PDF_S = (1 - prob) * out1.PDF_S + prob * out2.PDF_S;
		}
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession& session) const override
	{
		PR_PROFILE_THIS;

		const float prob = fresnelTermHero(in.Context.NdotV(), in.ShadingContext);

		if (in.RND.getFloat() < (1 - prob)) {
			mMaterials[0]->sample(in, out, session);

			out.Weight *= (1 - prob);
			out.PDF_S *= (1 - prob);
		} else {
			mMaterials[1]->sample(in, out, session);

			out.Weight *= prob;
			out.PDF_S *= prob;
		}

		if constexpr (SpectralVarying)
			out.Flags |= MSF_SpectralVarying;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << IMaterial::dumpInformation()
			   << "  <FresnelMaterial>:" << std::endl
			   << "    [0]: " << mMaterials[0]->dumpInformation() << std::endl
			   << "    [1]: " << mMaterials[1]->dumpInformation() << std::endl
			   << "    IOR: " << mIOR->dumpInformation() << std::endl;

		return stream.str();
	}

private:
	const std::array<std::shared_ptr<IMaterial>, 2> mMaterials;
	const std::shared_ptr<FloatSpectralNode> mIOR;
};

template <bool SpectralVarying>
std::shared_ptr<IMaterial> createMaterial(const SceneLoadContext& ctx)
{
	const ParameterGroup& params = ctx.parameters();
	const auto mat1				 = ctx.lookupMaterial(params.getParameter("material1"));
	const auto mat2				 = ctx.lookupMaterial(params.getParameter("material2"));
	const auto ior				 = ctx.lookupSpectralNode("ior", 1.55f);

	if (!mat1 || !mat2) {
		PR_LOG(L_ERROR) << "Valid material1 or material2 parameters for blend material missing" << std::endl;
		return nullptr;
	}

	int deltaCount = 0;
	if (mat1->hasOnlyDeltaDistribution())
		++deltaCount;
	if (mat2->hasOnlyDeltaDistribution())
		++deltaCount;

	switch (deltaCount) {
	case 0:
	default:
		return std::make_shared<FresnelBlendMaterial<MD_None, SpectralVarying>>(mat1, mat2, ior);
	case 1:
		if (mat1->hasOnlyDeltaDistribution())
			return std::make_shared<FresnelBlendMaterial<MD_First, SpectralVarying>>(mat1, mat2, ior);
		else
			return std::make_shared<FresnelBlendMaterial<MD_Second, SpectralVarying>>(mat1, mat2, ior);
	case 2:
		return std::make_shared<FresnelBlendMaterial<MD_All, SpectralVarying>>(mat1, mat2, ior);
	}
}

// Having a better system of fresnel stuff and plugins could allow to mix blend & fresnelblend
class FresnelBlendMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		const bool spectralVarying = ctx.parameters().getBool("spectral_varying", false);
		if (spectralVarying)
			return createMaterial<true>(ctx);
		else
			return createMaterial<false>(ctx);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "fresnelblend", "fresnelmix" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::FresnelBlendMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)