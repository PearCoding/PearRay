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

// TODO: Add ior branching
class GlassMaterial : public IMaterial {
public:
	GlassMaterial(uint32 id,
				  const std::shared_ptr<FloatSpectralShadingSocket>& alb,
				  const std::shared_ptr<FloatSpectralShadingSocket>& ior,
				  bool thin)
		: IMaterial(id)
		, mSpecularity(alb)
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

	inline float fresnelTerm(const ShadingPoint& spt, float& eta) const
	{
		// TODO
		//const auto ior = mIOR->eval(spt);
		float n1 = 1;
		//float n2 = ior[0];
		// https://refractiveindex.info/?shelf=glass&book=BK7&page=SCHOTT
		float n2 = Reflection::sellmeier(spt.Ray.WavelengthNM[0], 1.03961212, 0.231792344, 1.01046945, 0.00600069867, 0.0200179144, 103.560653);
		if (spt.Flags & SPF_Inside)
			std::swap(n1, n2);

		eta = n1 / n2;
		return Fresnel::dielectric(-spt.NdotV, n1, n2);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		out.Weight = mSpecularity->eval(in.Point);
		out.PDF_S  = 1;

		if (in.NdotL < 0)
			out.Type = MST_SpecularTransmission;
		else
			out.Type = MST_SpecularReflection;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		float eta;
		const float F = fresnelTerm(in.Point, eta);
		out.Weight	  = mSpecularity->eval(in.Point); // The weight is independent of the fresnel term

		if (in.RND[0] <= F) {
			out.Type	 = MST_SpecularReflection;
			out.Outgoing = Reflection::reflect(in.Point.NdotV, in.Point.N,
											   in.Point.Ray.Direction);
		} else {
			const float NdotT = Reflection::refraction_angle(in.Point.NdotV, eta);

			if (NdotT < 0) { // TOTAL REFLECTION
				if (mThin) { // Ignore
					out.PDF_S  = 0;
					out.Weight = SpectralBlob::Zero();
					return;
				} else {
					out.Type	 = MST_SpecularReflection;
					out.Outgoing = Reflection::reflect(-in.Point.NdotV, -in.Point.N, in.Point.Ray.Direction);
				}
			} else {
				out.Type	 = MST_SpecularTransmission;
				out.Outgoing = Reflection::refract(eta, in.Point.NdotV, NdotT,
												   in.Point.N, in.Point.Ray.Direction);
			}
		}

		out.PDF_S = 1;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <GlassMaterial>:" << std::endl
			   << "    Specularity: " << (mSpecularity ? mSpecularity->dumpInformation() : "NONE") << std::endl
			   << "    IOR: " << (mIOR ? mIOR->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralShadingSocket> mSpecularity;
	std::shared_ptr<FloatSpectralShadingSocket> mIOR;
	bool mThin;
};

class GlassMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;
		return std::make_shared<GlassMaterial>(id,
											   ctx.Env->lookupSpectralShadingSocket(params.getParameter("specularity"), 1),
											   ctx.Env->lookupSpectralShadingSocket(params.getParameter("index"), 1.55f),
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