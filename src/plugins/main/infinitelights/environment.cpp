#include "Environment.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightFactory.h"
#include "math/Projection.h"
#include "math/Spherical.h"
#include "math/Tangent.h"
#include "registry/Registry.h"
#include "sampler/Distribution2D.h"

namespace PR {
class EnvironmentLight : public IInfiniteLight {
public:
	EnvironmentLight(uint32 id, const std::string& name,
					 const std::shared_ptr<FloatSpectralMapSocket>& spec,
					 const std::shared_ptr<FloatSpectralMapSocket>& background,
					 float factor)
		: IInfiniteLight(id, name)
		, mRadiance(spec)
		, mBackground(background)
		, mRadianceFactor(factor)
	{
		Vector2i recSize = spec->queryRecommendedSize();
		if (recSize(0) > 1 && recSize(1) > 1) {
			mDistribution = std::make_unique<Distribution2D>(recSize(0), recSize(1));

			const Vector2f filterSize(1.0f / recSize(0), 1.0f / recSize(1));

			mDistribution->generate([&](size_t x, size_t y) {
				float u = x / (float)recSize(0);
				float v = 1 - y / (float)recSize(1);

				float sinTheta = std::sin(PR_PI * (y + 0.5f) / recSize(1));

				MapSocketCoord coord;
				coord.UV  = Vector2f(u, v);
				coord.dUV = filterSize;

				const float val = sinTheta * mRadiance->relativeLuminance(coord);
				return (val <= PR_EPSILON) ? 0.0f : val;
			});
		}
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		MapSocketCoord coord;
		coord.UV	= Spherical::uv_from_normal(in.Point.Ray.Direction);
		coord.UV(1) = 1 - coord.UV(1);
		coord.Index = in.Point.Ray.WavelengthIndex;

		if (in.Point.Ray.IterationDepth == 0)
			out.Weight = mBackground->eval(coord);
		else
			out.Weight = mRadiance->eval(coord);

		if (mDistribution) {
			const float sinTheta = std::sin(coord.UV(1) * PR_PI);
			const float denom	= 2 * PR_PI * PR_PI * sinTheta;
			out.PDF_S			 = (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
		} else {
			out.PDF_S = Projection::cos_hemi_pdf(std::abs(in.Point.N.dot(in.Point.Ray.Direction)));
		}
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		if (mDistribution) {
			Vector2f uv  = mDistribution->sampleContinuous(in.RND, out.PDF_S);
			out.Outgoing = Spherical::cartesian_from_uv(uv(0), uv(1));
			out.Outgoing = Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);

			MapSocketCoord coord;
			coord.UV	= uv;
			coord.UV(1) = 1 - coord.UV(1);
			coord.Index = in.Point.Ray.WavelengthIndex;
			out.Weight  = mRadianceFactor * mRadiance->eval(coord);

			const float sinTheta = std::sin((1 - uv(1)) * PR_PI);
			const float denom	= 2 * PR_PI * PR_PI * sinTheta;
			out.PDF_S			 = (denom <= PR_EPSILON) ? 0.0f : out.PDF_S / denom;
		} else {
			out.Outgoing = Projection::cos_hemi(in.RND[0], in.RND[1], out.PDF_S);
			out.Outgoing = Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);

			MapSocketCoord coord;
			coord.UV	= in.RND;
			coord.UV(1) = 1 - coord.UV(1);
			coord.Index = in.Point.Ray.WavelengthIndex;
			out.Weight  = mRadianceFactor * mRadiance->eval(coord);
		}
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <EnvironmentLight>:" << std::endl
			   << "    Radiance:     " << (mRadiance ? mRadiance->dumpInformation() : "NONE") << std::endl
			   << "    Background:   " << (mBackground ? mBackground->dumpInformation() : "NONE") << std::endl
			   << "    Factor:       " << mRadianceFactor << std::endl;
		if (mDistribution) {
			stream << "    Distribution: " << mDistribution->width() << "x" << mDistribution->height() << std::endl;
		} else {
			stream << "    Distribution: NONE" << std::endl;
		};

		return stream.str();
	}

private:
	std::unique_ptr<Distribution2D> mDistribution;

	// Radiance is used for sampling, background is used when a ray hits the background
	// Most of the time both are the same
	std::shared_ptr<FloatSpectralMapSocket> mRadiance;
	std::shared_ptr<FloatSpectralMapSocket> mBackground;
	float mRadianceFactor;
};

class EnvironmentLightFactory : public IInfiniteLightFactory {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, uint32 uuid, const Environment& env) override
	{
		const Registry& reg = env.registry();

		const std::string name			 = reg.getForObject<std::string>(RG_INFINITELIGHT, uuid,
																 "name", "__unknown");
		const std::string radianceName   = reg.getForObject<std::string>(RG_INFINITELIGHT, uuid,
																		 "radiance", "");
		const std::string backgroundName = reg.getForObject<std::string>(RG_INFINITELIGHT, uuid,
																		 "background", "");
		const float factor				 = std::max(0.0000001f, reg.getForObject<float>(RG_INFINITELIGHT, uuid,
																			"factor", 1.0f));

		auto rad = env.getSpectralMapSocket(radianceName, 1);
		std::shared_ptr<FloatSpectralMapSocket> background;
		if (backgroundName.empty())
			background = rad;
		else
			background = env.getSpectralMapSocket(backgroundName, 1);

		return std::make_shared<EnvironmentLight>(id, name, rad, background, factor);
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "env", "environment", "background" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::EnvironmentLightFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)