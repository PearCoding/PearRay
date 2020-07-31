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

template <bool HasTransmissionColor, bool IsThin>
class DielectricMaterial : public IMaterial {
public:
	DielectricMaterial(uint32 id,
				  const std::shared_ptr<FloatSpectralNode>& spec,
				  const std::shared_ptr<FloatSpectralNode>& trans,
				  const std::shared_ptr<FloatSpectralNode>& ior)
		: IMaterial(id)
		, mSpecularity(spec)
		, mTransmission(trans)
		, mIOR(ior)
	{
	}

	virtual ~DielectricMaterial() = default;

	void startGroup(size_t, const RenderTileSession&) override
	{
	}

	void endGroup() override
	{
	}

	int flags() const override { return MF_DeltaDistribution | (IsThin ? 0 : MF_SpectralVarying); }

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

		if (in.RND[0] <= F) {
			out.Type = MST_SpecularReflection;
			out.L	 = Reflection::reflect(in.Context.V);
		} else {
			if constexpr (IsThin) {
				out.Type = MST_SpecularTransmission;
				out.L	 = in.Context.V;
				if constexpr (HasTransmissionColor)
					out.Weight = mTransmission->eval(in.ShadingContext);
			} else {
				const float NdotT = Reflection::refraction_angle(in.Context.NdotV(), eta);

				if (NdotT < 0) { // TOTAL REFLECTION
					out.Type = MST_SpecularReflection;
					out.L	 = Reflection::reflect(in.Context.V);
				} else {
					out.Type = MST_SpecularTransmission;
					out.L	 = Reflection::refract(eta, NdotT, in.Context.V);

					if constexpr (HasTransmissionColor)
						out.Weight = mTransmission->eval(in.ShadingContext);
				}
			}
		}

		out.PDF_S = 1;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <DielectricMaterial>:" << std::endl
			   << "    Specularity:  " << mSpecularity->dumpInformation() << std::endl;

		if constexpr (HasTransmissionColor)
			stream << "    Transmission: " << mTransmission->dumpInformation() << std::endl;

		stream << "    IOR:          " << mIOR->dumpInformation() << std::endl;
		if constexpr (IsThin)
			stream << "    IsThin:       true" << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralNode> mSpecularity;
	std::shared_ptr<FloatSpectralNode> mTransmission;
	std::shared_ptr<FloatSpectralNode> mIOR;
};

template <bool HasTransmissionColor, bool IsThin>
static std::shared_ptr<IMaterial> createMaterial(uint32 id, const SceneLoadContext& ctx)
{
	return std::make_shared<DielectricMaterial<HasTransmissionColor, IsThin>>(
		id,
		ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("specularity"), 1),
		HasTransmissionColor ? ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("transmission"), 1) : nullptr,
		ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("index"), 1.55f));
}

class DielectricMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		if (ctx.Parameters.hasParameter("transmission")) {
			if (ctx.Parameters.getBool("thin", false))
				return createMaterial<true, true>(id, ctx);
			else
				return createMaterial<true, false>(id, ctx);
		} else {
			if (ctx.Parameters.getBool("thin", false))
				return createMaterial<false, true>(id, ctx);
			else
				return createMaterial<false, false>(id, ctx);
		}
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "glass", "dielectric" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::DielectricMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)