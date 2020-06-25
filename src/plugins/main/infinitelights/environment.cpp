#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "math/Projection.h"
#include "math/Spherical.h"
#include "math/Tangent.h"

#include "sampler/Distribution2D.h"

namespace PR {
// Four variants available
// -> Distribution + Split
// -> Distribution
// -> Split
// -> None

class EnvironmentLightBase : public IInfiniteLight {
public:
	EnvironmentLightBase(uint32 id, const std::string& name,
						 const std::shared_ptr<FloatSpectralMapSocket>& spec,
						 const std::shared_ptr<FloatSpectralMapSocket>& background,
						 const std::shared_ptr<Distribution2D>& distribution,
						 float factor,
						 const Eigen::Matrix3f& trans)
		: IInfiniteLight(id, name)
		, mDistribution(distribution)
		, mRadiance(spec)
		, mBackground(background)
		, mRadianceFactor(factor)
		, mTransform(trans)
		, mInvTransform(trans.inverse())
	{
	}

	inline void evalSD(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out) const
	{
		MapSocketCoord coord;
		Vector3f dir = mInvTransform * in.Ray.Direction;

		coord.UV		   = Spherical::uv_from_normal(dir);
		coord.UV(1)		   = 1 - coord.UV(1);
		coord.WavelengthNM = in.Ray.WavelengthNM;

		if (in.Ray.IterationDepth == 0)
			out.Weight = mBackground->eval(coord);
		else
			out.Weight = mRadianceFactor * mRadiance->eval(coord);

		const float sinTheta = std::sin(coord.UV(1) * PR_PI);
		const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
		out.PDF_S			 = (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
	}

	inline void evalS(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out) const
	{
		MapSocketCoord coord;
		Vector3f dir = mInvTransform * in.Ray.Direction;

		coord.UV		   = Spherical::uv_from_normal(dir);
		coord.UV(1)		   = 1 - coord.UV(1);
		coord.WavelengthNM = in.Ray.WavelengthNM;

		if (in.Ray.IterationDepth == 0)
			out.Weight = mBackground->eval(coord);
		else
			out.Weight = mRadianceFactor * mRadiance->eval(coord);

		out.PDF_S = in.Point ? Projection::cos_hemi_pdf(std::abs(in.Point->Surface.N.dot(dir))) : 1.0f;
	}

	inline void evalD(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out) const
	{
		MapSocketCoord coord;
		Vector3f dir = mInvTransform * in.Ray.Direction;

		coord.UV		   = Spherical::uv_from_normal(dir);
		coord.UV(1)		   = 1 - coord.UV(1);
		coord.WavelengthNM = in.Ray.WavelengthNM;

		out.Weight = mRadianceFactor * mRadiance->eval(coord);

		const float sinTheta = std::sin(coord.UV(1) * PR_PI);
		const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
		out.PDF_S			 = (denom <= PR_EPSILON) ? 0.0f : 1.0f / denom;
	}

	inline void evalN(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out) const
	{
		MapSocketCoord coord;
		Vector3f dir = mInvTransform * in.Ray.Direction;

		coord.UV		   = Spherical::uv_from_normal(dir);
		coord.UV(1)		   = 1 - coord.UV(1);
		coord.WavelengthNM = in.Ray.WavelengthNM;

		out.Weight = mRadianceFactor * mRadiance->eval(coord);

		out.PDF_S = in.Point ? Projection::cos_hemi_pdf(std::abs(in.Point->Surface.N.dot(dir))) : 1.0f;
	}

	inline void sampleD(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out) const
	{
		Vector2f uv	 = mDistribution->sampleContinuous(in.RND, out.PDF_S);
		out.Outgoing = Spherical::cartesian_from_uv(uv(0), uv(1));
		out.Outgoing = mTransform * Tangent::fromTangentSpace(in.Point.Surface.N, in.Point.Surface.Nx, in.Point.Surface.Ny, out.Outgoing);

		MapSocketCoord coord;
		coord.UV		   = uv;
		coord.UV(1)		   = 1 - coord.UV(1);
		coord.WavelengthNM = in.Point.Ray.WavelengthNM;
		out.Weight		   = mRadianceFactor * mRadiance->eval(coord);

		const float sinTheta = std::sin((1 - uv(1)) * PR_PI);
		const float denom	 = 2 * PR_PI * PR_PI * sinTheta;
		out.PDF_S			 = (denom <= PR_EPSILON) ? 0.0f : out.PDF_S / denom;
	}

	inline void sampleN(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out) const
	{
		out.Outgoing = Projection::cos_hemi(in.RND[0], in.RND[1], out.PDF_S);
		out.Outgoing = mTransform * Tangent::fromTangentSpace(in.Point.Surface.N, in.Point.Surface.Nx, in.Point.Surface.Ny, out.Outgoing);

		MapSocketCoord coord;
		coord.UV		   = in.RND;
		coord.UV(1)		   = 1 - coord.UV(1);
		coord.WavelengthNM = in.Point.Ray.WavelengthNM;
		out.Weight		   = mRadianceFactor * mRadiance->eval(coord);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IInfiniteLight::dumpInformation()
			   << "  <EnvironmentDSLight>:" << std::endl
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
	const std::shared_ptr<Distribution2D> mDistribution;

	// Radiance is used for sampling, background is used when a ray hits the background
	// Most of the time both are the same
	const std::shared_ptr<FloatSpectralMapSocket> mRadiance;
	const std::shared_ptr<FloatSpectralMapSocket> mBackground;
	const float mRadianceFactor;
	const Eigen::Matrix3f mTransform;
	const Eigen::Matrix3f mInvTransform;
};

class EnvironmentSDLight : public EnvironmentLightBase {
public:
	EnvironmentSDLight(uint32 id, const std::string& name,
					   const std::shared_ptr<FloatSpectralMapSocket>& spec,
					   const std::shared_ptr<FloatSpectralMapSocket>& background,
					   const std::shared_ptr<Distribution2D>& dist,
					   float factor,
					   const Eigen::Matrix3f& trans)
		: EnvironmentLightBase(id, name, spec, background, dist, factor, trans)
	{
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		evalSD(in, out);
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		sampleD(in, out);
	}
};

class EnvironmentSLight : public EnvironmentLightBase {
public:
	EnvironmentSLight(uint32 id, const std::string& name,
					  const std::shared_ptr<FloatSpectralMapSocket>& spec,
					  const std::shared_ptr<FloatSpectralMapSocket>& background,
					  float factor,
					  const Eigen::Matrix3f& trans)
		: EnvironmentLightBase(id, name, spec, background, nullptr, factor, trans)
	{
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		evalS(in, out);
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		sampleN(in, out);
	}
};

class EnvironmentDLight : public EnvironmentLightBase {
public:
	EnvironmentDLight(uint32 id, const std::string& name,
					  const std::shared_ptr<FloatSpectralMapSocket>& spec,
					  const std::shared_ptr<Distribution2D>& dist,
					  float factor,
					  const Eigen::Matrix3f& trans)
		: EnvironmentLightBase(id, name, spec, spec, dist, factor, trans)
	{
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		evalD(in, out);
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		sampleD(in, out);
	}
};

class EnvironmentNLight : public EnvironmentLightBase {
public:
	EnvironmentNLight(uint32 id, const std::string& name,
					  const std::shared_ptr<FloatSpectralMapSocket>& spec,
					  float factor,
					  const Eigen::Matrix3f& trans)
		: EnvironmentLightBase(id, name, spec, spec, nullptr, factor, trans)
	{
	}

	void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out,
			  const RenderTileSession&) const override
	{
		evalN(in, out);
	}

	void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out,
				const RenderTileSession&) const override
	{
		sampleN(in, out);
	}
};

class EnvironmentLightFactory : public IInfiniteLightPlugin {
public:
	std::shared_ptr<IInfiniteLight> create(uint32 id, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.Parameters;

		const std::string name = params.getString("name", "__unknown");
		const float factor	   = std::max(0.0000001f, params.getNumber("factor", 1.0f));

		auto radP		 = params.getParameter("radiance");
		auto backgroundP = params.getParameter("background");

		std::shared_ptr<FloatSpectralMapSocket> radiance;
		std::shared_ptr<FloatSpectralMapSocket> background;
		if (radP.isValid() && backgroundP.isValid()) {
			radiance   = ctx.Env->lookupSpectralMapSocket(radP, 1);
			background = ctx.Env->lookupSpectralMapSocket(backgroundP, 1);
		} else if (radP.isValid()) {
			radiance   = ctx.Env->lookupSpectralMapSocket(radP, 1);
			background = radiance;
		} else {
			background = ctx.Env->lookupSpectralMapSocket(backgroundP, 1);
			radiance   = background;
		}

		Eigen::Matrix3f trans = params.getMatrix3f("orientation", Eigen::Matrix3f::Identity());

		Vector2i recSize = radiance->queryRecommendedSize();
		std::shared_ptr<Distribution2D> dist;
		if (recSize(0) > 1 && recSize(1) > 1) {
			dist = std::make_shared<Distribution2D>(recSize(0), recSize(1));

			const Vector2f filterSize(1.0f / recSize(0), 1.0f / recSize(1));

			PR_LOG(L_INFO) << "Generating 2d environment distribution of " << name << std::endl;

			dist->generate([&](size_t x, size_t y) {
				float u = x / (float)recSize(0);
				float v = 1 - y / (float)recSize(1);

				float sinTheta = std::sin(PR_PI * (y + 0.5f) / recSize(1));

				MapSocketCoord coord;
				coord.UV		   = Vector2f(u, v);
				coord.dUV		   = filterSize;
				coord.WavelengthNM = SpectralBlob(560.0f, 540.0f, 400.0f, 600.0f); // Preset of wavelengths to test

				const float val = sinTheta * radiance->eval(coord).maxCoeff();
				return (val <= PR_EPSILON) ? 0.0f : val;
			});
		}

		if (dist) {
			if (radiance == background)
				return std::make_shared<EnvironmentDLight>(id, name, radiance, dist, factor, trans);
			else
				return std::make_shared<EnvironmentSDLight>(id, name, radiance, background, dist, factor, trans);
		} else {
			if (radiance == background)
				return std::make_shared<EnvironmentNLight>(id, name, radiance, factor, trans);
			else
				return std::make_shared<EnvironmentSLight>(id, name, radiance, background, factor, trans);
		}
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