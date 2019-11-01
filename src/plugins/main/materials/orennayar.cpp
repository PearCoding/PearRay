#include "Environment.h"
#include "Logger.h"
#include "material/IMaterial.h"
#include "material/IMaterialFactory.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "renderer/RenderContext.h"
#include "shader/ConstShadingSocket.h"
#include "shader/ShadingSocket.h"

#include <sstream>

namespace PR {

class OrenNayarMaterial : public IMaterial {
public:
	OrenNayarMaterial(uint32 id,
					  const std::shared_ptr<FloatSpectralShadingSocket>& alb,
					  const std::shared_ptr<FloatSpectralShadingSocket>& rough)
		: IMaterial(id)
		, mAlbedo(alb)
		, mRoughness(rough)
	{
	}

	virtual ~OrenNayarMaterial() = default;

	void startGroup(size_t, const RenderTileSession&) override
	{
	}

	void endGroup() override
	{
	}

	// https://mimosa-pudica.net/improved-oren-nayar.html
	inline float calc(const Vector3f& L, float NdotL, const ShadingPoint& spt) const
	{
		float roughness = mRoughness->eval(spt);
		roughness *= roughness;

		float weight = mAlbedo->eval(spt);

		if (roughness > PR_EPSILON) {
			const float s = NdotL * spt.NdotV - L.dot(spt.Ray.Direction);
			const float t = s < PR_EPSILON ? 1.0f : std::max(NdotL, -spt.NdotV);

			const float A = 1 - 0.5f * roughness / (roughness + 0.33f) + 0.17f * weight * roughness / (roughness + 0.13f);
			const float B = 0.45f * roughness / (roughness + 0.09f);
			weight *= (A + B * s / t);
		}

		return weight * PR_1_PI * std::abs(NdotL);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		out.Weight		   = calc(in.Outgoing, std::abs(in.NdotL), in.Point);
		out.Type		   = MST_DiffuseReflection;
		out.PDF_S  = Projection::cos_hemi_pdf(std::abs(in.NdotL));
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		float pdf;
		out.Outgoing = Projection::cos_hemi(in.RND[0], in.RND[1], pdf);
		out.Outgoing = Tangent::align(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);

		float NdotL		   = std::abs(out.Outgoing.dot(in.Point.N));
		out.Weight		   = calc(out.Outgoing, NdotL, in.Point);
		out.Type		   = MST_DiffuseReflection;
		out.PDF_S  = pdf;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <OrenNayarMaterial>:" << std::endl
			   << "    Albedo: " << (mAlbedo ? mAlbedo->dumpInformation() : "NONE") << std::endl
			   << "    Roughness: " << (mRoughness ? mRoughness->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

protected:
	void onFreeze(RenderContext*) override
	{
	}

private:
	std::shared_ptr<FloatSpectralShadingSocket> mAlbedo;
	std::shared_ptr<FloatSpectralShadingSocket> mRoughness;
};

class OrenNayarMaterialFactory : public IMaterialFactory {
public:
	std::shared_ptr<IMaterial> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const Registry& reg = env.registry();

		const std::string albedoName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "albedo", "");

		float roughness
			= reg.getForObject<float>(RG_MATERIAL, uuid, "roughness", 0.0f);
		std::shared_ptr<FloatSpectralShadingSocket> roughnessS
			= std::make_shared<ConstSpectralShadingSocket>(Spectrum(env.spectrumDescriptor(), roughness));

		return std::make_shared<OrenNayarMaterial>(id,
												   env.getSpectralShadingSocket(albedoName, 1),
												   roughnessS);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "orennayar", "oren", "rough" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::OrenNayarMaterialFactory, "mat_orennayar", PR_PLUGIN_VERSION)