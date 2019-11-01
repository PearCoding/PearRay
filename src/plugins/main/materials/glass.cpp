#include "Environment.h"
#include "material/IMaterial.h"
#include "material/IMaterialFactory.h"
#include "math/Fresnel.h"
#include "math/Projection.h"
#include "math/Reflection.h"
#include "renderer/RenderContext.h"
#include "shader/ConstShadingSocket.h"
#include "shader/ShadingSocket.h"

#include <sstream>

namespace PR {

class GlassMaterial : public IMaterial {
public:
	GlassMaterial(uint32 id,
				  const std::shared_ptr<FloatSpectralShadingSocket>& alb,
				  const std::shared_ptr<FloatSpectralShadingSocket>& ior)
		: IMaterial(id)
		, mSpecularity(alb)
		, mIOR(ior)
	{
	}

	virtual ~GlassMaterial() = default;

	void startGroup(size_t, const RenderTileSession&) override
	{
	}

	void endGroup() override
	{
	}

	inline float fresnelTerm(const ShadingPoint& spt, float& eta) const
	{
		float n1 = 1;
		float n2 = mIOR->eval(spt);
		if (spt.Flags & SPF_Inside)
			std::swap(n1, n2);

		eta = n1 / n2;
		return Fresnel::dielectric(spt.NdotV, n1, n2);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		out.Weight		   = mSpecularity->eval(in.Point);
		out.PDF_S  = std::numeric_limits<float>::infinity();

		if (in.NdotL < 0)
			out.Type = MST_SpecularTransmission;
		else
			out.Type = MST_SpecularReflection;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		float eta;
		const float F = fresnelTerm(in.Point, eta);
		out.Weight	= mSpecularity->eval(in.Point); // The weight is independent of the fresnel term

		if (in.RND[0] <= F) {
			out.Type	 = MST_SpecularReflection;
			out.Outgoing = Reflection::reflect(in.Point.NdotV, in.Point.N,
											   in.Point.Ray.Direction);
		} else {
			const float NdotT = Reflection::refraction_angle(in.Point.NdotV, eta);

			if (NdotT < 0) { // TOTAL REFLECTION
				out.Type	 = MST_SpecularReflection;
				out.Outgoing = Reflection::reflect(-in.Point.NdotV, -in.Point.N, in.Point.Ray.Direction);
			} else {
				out.Type	 = MST_SpecularTransmission;
				out.Outgoing = Reflection::refract(eta, in.Point.NdotV, NdotT,
												   in.Point.N, in.Point.Ray.Direction);
			}
		}

		out.PDF_S  = std::numeric_limits<float>::infinity();
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

protected:
	void onFreeze(RenderContext*) override
	{
	}

private:
	std::shared_ptr<FloatSpectralShadingSocket> mSpecularity;
	std::shared_ptr<FloatSpectralShadingSocket> mIOR;
};

class GlassMaterialFactory : public IMaterialFactory {
public:
	std::shared_ptr<IMaterial> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const Registry& reg = env.registry();

		const std::string specName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "specularity", "");

		const std::string iorName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "index", "");

		return std::make_shared<GlassMaterial>(id,
											   env.getSpectralShadingSocket(specName, 1),
											   env.getSpectralShadingSocket(iorName, 1.55f));
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

PR_PLUGIN_INIT(PR::GlassMaterialFactory, "mat_glass", PR_PLUGIN_VERSION)