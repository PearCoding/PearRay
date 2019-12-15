#include "Environment.h"
#include "Profiler.h"
#include "material/IMaterial.h"
#include "material/IMaterialFactory.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

class LambertMaterial : public IMaterial {
public:
	LambertMaterial(uint32 id, const std::shared_ptr<FloatSpectralShadingSocket>& alb)
		: IMaterial(id)
		, mAlbedo(alb)
	{
	}

	virtual ~LambertMaterial() = default;

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

		const float dot = std::max(0.0f, in.NdotL);
		out.Weight		= mAlbedo->eval(in.Point) * dot * PR_1_PI;
		out.PDF_S		= Projection::cos_hemi_pdf(dot);
		out.Type		= MST_DiffuseReflection;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		float pdf;
		out.Outgoing = Projection::cos_hemi(in.RND[0], in.RND[1], pdf);
		out.Outgoing = Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);

		out.Weight = mAlbedo->eval(in.Point) * std::max(0.0f, in.Point.N.dot(out.Outgoing)) * PR_1_PI;
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
	std::shared_ptr<FloatSpectralShadingSocket> mAlbedo;
};

class LambertMaterialFactory : public IMaterialFactory {
public:
	std::shared_ptr<IMaterial> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const Registry& reg = env.registry();

		const std::string albedoName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "albedo", "");

		return std::make_shared<LambertMaterial>(id, env.getSpectralShadingSocket(albedoName, 1));
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

PR_PLUGIN_INIT(PR::LambertMaterialFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)