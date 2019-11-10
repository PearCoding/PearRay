#include "Environment.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightFactory.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "registry/Registry.h"
#include "sampler/Distribution2D.h"

namespace PR {
class EnvironmentLight : public IInfiniteLight {
public:
	EnvironmentLight(uint32 id, const std::string& name,
					 const std::shared_ptr<FloatSpectralMapSocket>& spec)
		: IInfiniteLight(id, name)
		, mRadiance(spec)
	{
		Vector2i recSize = spec->queryRecommendedSize();
		if (recSize(0) > 1 && recSize(1) > 1) {
			mDistribution = std::make_unique<Distribution2D>(recSize(0), recSize(1));

			const Vector2f filterSize(1.0f / recSize(0), 1.0f / recSize(1));

			mDistribution->generate([&](size_t x, size_t y) {
				float u = x / (float)recSize(0);
				float v = y / (float)recSize(1);

				float sinTheta = std::sin(PR_PI * (y + 0.5f) / recSize(1));

				MapSocketCoord coord;
				coord.UV  = Vector2f(u, v);
				coord.dUV = filterSize;

				return sinTheta * mRadiance->relativeLuminance(coord);
			});
		}
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		MapSocketCoord coord;
		coord.UV	= Spherical::uv_from_normal(in.Point.Ray.Direction);
		coord.Index = in.Point.Ray.WavelengthIndex;

		out.Weight = mRadiance->eval(coord);
		out.PDF_S  = 0.5f * PR_1_PI;
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
			coord.Index = in.Point.Ray.WavelengthIndex;
			out.Weight  = mRadiance->eval(coord);

			const float sinTheta = std::sin(uv(1) * PR_PI);
			if(sinTheta > PR_EPSILON)
				out.PDF_S = 0.0f;
			else
				out.PDF_S /= (2 * PR_PI * PR_PI * sinTheta);
		} else {
			out.Outgoing = Projection::hemi(in.RND[0], in.RND[1], out.PDF_S);
			out.Outgoing = Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);

			MapSocketCoord coord;
			coord.UV	= in.RND;
			coord.Index = in.Point.Ray.WavelengthIndex;
			out.Weight  = mRadiance->eval(coord);
		}
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <EnvironmentLight>:" << std::endl
			   << "    Radiance:     " << (mRadiance ? mRadiance->dumpInformation() : "NONE") << std::endl;
		if (mDistribution) {
			stream << "    Distribution: " << mDistribution->width() << "x" << mDistribution->height() << std::endl;
		} else {
			stream << "    Distribution: NONE" << std::endl;
		};

		return stream.str();
	}

protected:
	void onFreeze(RenderContext* context) override
	{
		VirtualEntity::onFreeze(context);
	}

private:
	std::unique_ptr<Distribution2D> mDistribution;
	std::shared_ptr<FloatSpectralMapSocket> mRadiance;
};

class EnvironmentLightFactory : public IInfiniteLightFactory {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, uint32 uuid, const Environment& env) override
	{
		const Registry& reg = env.registry();

		const std::string name		   = reg.getForObject<std::string>(RG_INFINITELIGHT, uuid,
															   "name", "__unknown");
		const std::string radianceName = reg.getForObject<std::string>(RG_INFINITELIGHT, uuid,
																	   "radiance", "");

		return std::make_shared<EnvironmentLight>(id, name, env.getSpectralMapSocket(radianceName, 1));
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

PR_PLUGIN_INIT(PR::EnvironmentLightFactory, "inf_environment", PR_PLUGIN_VERSION)