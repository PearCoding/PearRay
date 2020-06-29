#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Projection.h"
#include "math/Reflection.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

template <bool HasTransmissionColor>
class GlassMaterial : public IMaterial {
public:
	GlassMaterial(uint32 id,
				  const std::shared_ptr<FloatSpectralNode>& spec,
				  const std::shared_ptr<FloatSpectralNode>& trans,
				  const std::shared_ptr<FloatSpectralNode>& ior,
				  bool thin)
		: IMaterial(id)
		, mSpecularity(spec)
		, mTransmission(trans)
		, mIOR(ior)
		, mThin(thin)
	{
	}

	virtual ~GlassMaterial() = default;

	void startGroup(size_t, const RenderTileSession&) override
	{
	}

	void endGroup() override
	{
	}

	int flags() const override { return MF_DeltaDistribution | MF_SpectralVarying; }

	inline SpectralBlob fresnelTerm(const MaterialSampleContext& spt, const ShadingContext& sctx) const
	{
		SpectralBlob n1 = SpectralBlob::Ones();
		SpectralBlob n2 = mIOR->eval(sctx);

		if (spt.IsInside)
			std::swap(n1, n2);

		SpectralBlob res;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			res[i] = Fresnel::dielectric(spt.NdotV(), n1[i], n2[i]);
		return res;
	}

	inline float fresnelTermHero(const MaterialSampleContext& spt, const ShadingContext& sctx, float& eta) const
	{
		SpectralBlob n1 = SpectralBlob::Ones();
		SpectralBlob n2 = mIOR->eval(sctx);

		if (spt.IsInside)
			std::swap(n1, n2);

		eta = n1[0] / n2[0];

		return Fresnel::dielectric(spt.NdotV(), n1[0], n2[0]);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		if constexpr (HasTransmissionColor) {
			SpectralBlob spec  = mSpecularity->eval(in.ShadingContext);
			SpectralBlob trans = mTransmission->eval(in.ShadingContext);
			SpectralBlob F	   = fresnelTerm(in.Context, in.ShadingContext);
			out.Weight		   = spec * F + trans * (1 - F);
		} else {
			out.Weight = mSpecularity->eval(in.ShadingContext);
		}

		out.PDF_S = 1;

		if (in.Context.NdotL() < 0)
			out.Type = MST_SpecularTransmission;
		else
			out.Type = MST_SpecularReflection;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		float eta;
		const float F = fresnelTermHero(in.Context, in.ShadingContext, eta);

		out.Weight = mSpecularity->eval(in.ShadingContext); // The weight is independent of the fresnel term
		SpectralBlob trans;
		if constexpr (HasTransmissionColor)
			trans = mTransmission->eval(in.ShadingContext);

		if (in.RND[0] <= F) {
			out.Type = MST_SpecularReflection;
			out.L	 = Reflection::reflect(in.Context.V);
		} else {
			const float NdotT = Reflection::refraction_angle(in.Context.NdotV(), eta);

			if (NdotT < 0) { // TOTAL REFLECTION
				if (mThin) { // Ignore
					out.PDF_S  = 0;
					out.Weight = SpectralBlob::Zero();
					return;
				} else {
					out.Type = MST_SpecularReflection;
					out.L	 = Reflection::reflect(in.Context.V);
				}
			} else {
				out.Type = MST_SpecularTransmission;
				out.L	 = Reflection::refract(eta, NdotT, in.Context.V);

				if constexpr (HasTransmissionColor)
					out.Weight = trans;
			}
		}

		out.PDF_S = 1;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <GlassMaterial>:" << std::endl
			   << "    Specularity: " << mSpecularity->dumpInformation() << std::endl;

		if constexpr (HasTransmissionColor)
			stream << "    Transmission: " << mTransmission->dumpInformation() << std::endl;

		stream << "    IOR: " << mIOR->dumpInformation() << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralNode> mSpecularity;
	std::shared_ptr<FloatSpectralNode> mTransmission;
	std::shared_ptr<FloatSpectralNode> mIOR;
	bool mThin;
};

class GlassMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;

		if (ctx.Parameters.hasParameter("transmission"))
			return std::make_shared<GlassMaterial<true>>(id,
														 ctx.Env->lookupSpectralNode(params.getParameter("specularity"), 1),
														 ctx.Env->lookupSpectralNode(params.getParameter("transmission"), 1),
														 ctx.Env->lookupSpectralNode(params.getParameter("index"), 1.55f),
														 params.getBool("thin", false));
		else
			return std::make_shared<GlassMaterial<false>>(id,
														  ctx.Env->lookupSpectralNode(params.getParameter("specularity"), 1),
														  nullptr,
														  ctx.Env->lookupSpectralNode(params.getParameter("index"), 1.55f),
														  params.getBool("thin", false));
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "glass" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::GlassMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)