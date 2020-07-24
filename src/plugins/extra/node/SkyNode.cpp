#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "math/Spherical.h"
#include "renderer/RenderContext.h"
#include "shader/INodePlugin.h"

#include "skysun/SkyModel.h"
#include "skysun/SunLocation.h"

namespace PR {
// Sky texture!
class SkyNode : public FloatSpectralNode {
public:
	explicit SkyNode(const SkyModel& model)
		: mModel(model)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		SpectralBlob blob;
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const float f	= std::min<float>(AR_SPECTRAL_BANDS - 2, std::max(0.0f, (ctx.WavelengthNM[i] - AR_SPECTRAL_START) / AR_SPECTRAL_DELTA));
			const int index = f;
			const float t	= f - index;

			float radiance = mModel.radiance(index, ctx.UV(1), ctx.UV(0)) * (1 - t) + mModel.radiance(index + 1, ctx.UV(1), ctx.UV(0)) * t;
			blob[i]		   = radiance;
		}
		return blob;
	}

	Vector2i queryRecommendedSize() const override
	{
		return Vector2i(mModel.phiCount(), mModel.thetaCount());
	}
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "SkyNode (" << mModel.phiCount() << "x" << mModel.thetaCount() << ")"; // TODO: Better information?
		return sstream.str();
	}

private:
	const SkyModel mModel;
};

class SkyPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(uint32, const std::string&, const SceneLoadContext& ctx) override
	{
		auto ground_albedo	   = ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("albedo"), 0.2f);
		ElevationAzimuth sunEA = computeSunEA(ctx.Parameters);
		PR_LOG(L_INFO) << "Sun: " << sunEA.Elevation << " " << sunEA.Azimuth << std::endl;

		return std::make_shared<SkyNode>(SkyModel(ground_albedo, sunEA, ctx.Parameters));
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "sky" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SkyPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)