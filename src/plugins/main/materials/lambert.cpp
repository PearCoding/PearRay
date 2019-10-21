#include "Environment.h"
#include "material/IMaterial.h"
#include "material/IMaterialFactory.h"
#include "math/Projection.h"
#include "renderer/RenderContext.h"
#include "shader/ConstShadingSocket.h"
#include "shader/ShadingSocket.h"

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

	void startGroup(size_t size, const RenderTileSession& session) override
	{
	}

	void endGroup() override
	{
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession& session) const override
	{
		float NdotL		   = std::abs(in.Outgoing.dot(in.Point.N));
		out.Weight		   = mAlbedo->eval(in.Point) * PR_1_PI;
		out.PDF_S_Forward  = Projection::cos_hemi_pdf(NdotL);
		out.PDF_S_Backward = Projection::cos_hemi_pdf(NdotL);
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession& session) const override
	{
		float pdf;
		out.Outgoing	   = Projection::cos_hemi(in.RND[0], in.RND[1], pdf);
		out.Weight		   = mAlbedo->eval(in.Point) * PR_1_PI;
		out.Type		   = MST_DiffuseReflection;
		out.PDF_S_Backward = pdf;
		out.PDF_S_Forward  = pdf;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <DiffuseMaterial>:" << std::endl
			   << "    Albedo: " << (mAlbedo ? mAlbedo->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

protected:
	void onFreeze(RenderContext* context) override
	{
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

PR_PLUGIN_INIT(PR::LambertMaterialFactory, "mat_lambert", PR_PLUGIN_VERSION)