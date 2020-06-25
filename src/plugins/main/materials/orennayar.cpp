#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Projection.h"
#include "math/Tangent.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

class OrenNayarMaterial : public IMaterial {
public:
	OrenNayarMaterial(uint32 id,
					  const std::shared_ptr<FloatSpectralShadingSocket>& alb,
					  const std::shared_ptr<FloatScalarShadingSocket>& rough)
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
	inline SpectralBlob calc(const Vector3f& L, float NdotL, const ShadingPoint& spt) const
	{
		float roughness = mRoughness->eval(spt);
		roughness *= roughness;

		SpectralBlob weight = mAlbedo->eval(spt);

		if (roughness > PR_EPSILON) {
			const float s = NdotL * spt.Surface.NdotV - L.dot(spt.Ray.Direction);
			const float t = s < PR_EPSILON ? 1.0f : std::max(NdotL, -spt.Surface.NdotV);

			const SpectralBlob A = SpectralBlob::Ones() * (1 - 0.5f * roughness / (roughness + 0.33f))
								   + 0.17f * weight * roughness / (roughness + 0.13f);
			const float B = 0.45f * roughness / (roughness + 0.09f);
			weight *= A + SpectralBlob::Ones() * (B * s / t);
		}

		return weight * PR_1_PI * std::abs(NdotL);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		out.Weight = calc(in.Outgoing, std::abs(in.NdotL), in.Point);
		out.Type   = MST_DiffuseReflection;
		out.PDF_S  = Projection::cos_hemi_pdf(std::abs(in.NdotL));
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		float pdf;
		out.Outgoing = Projection::cos_hemi(in.RND[0], in.RND[1], pdf);
		out.Outgoing = Tangent::fromTangentSpace(in.Point.Surface.N, in.Point.Surface.Nx, in.Point.Surface.Ny, out.Outgoing);

		float NdotL = std::abs(out.Outgoing.dot(in.Point.Surface.N));
		out.Weight  = calc(out.Outgoing, NdotL, in.Point);
		out.Type	= MST_DiffuseReflection;
		out.PDF_S   = pdf;
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

private:
	std::shared_ptr<FloatSpectralShadingSocket> mAlbedo;
	std::shared_ptr<FloatScalarShadingSocket> mRoughness;
};

class OrenNayarMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;
		return std::make_shared<OrenNayarMaterial>(id,
												   ctx.Env->lookupSpectralShadingSocket(params.getParameter("albedo"), 1),
												   ctx.Env->lookupScalarShadingSocket(params.getParameter("roughness"), 0.5f));
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

PR_PLUGIN_INIT(PR::OrenNayarMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)