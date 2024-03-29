#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "integrator/IIntegratorPlugin.h"
#include "math/Sampling.h"
#include "math/Tangent.h"

#include "path/LightPath.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "renderer/StreamPipeline.h"
#include "trace/IntersectionPoint.h"

#include "Logger.h"

namespace PR {
constexpr RayFlags UsedRayType = RayFlag::Bounce; // Do not use RayFlags::Shadow, as most lights will be visible. We don't want that
class IntAOInstance : public IIntegratorInstance {
public:
	explicit IntAOInstance(size_t sample_count)
		: mSampleCount(sample_count)
	{
	}

	virtual ~IntAOInstance() = default;

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& grp)
	{
		const LightPath stdPath = LightPath::createCDL(1);

		session.tile()->statistics().add(RenderStatisticEntry::EntityHitCount, grp.size());
		session.tile()->statistics().add(RenderStatisticEntry::CameraDepthCount, grp.size());
		for (size_t i = 0; i < grp.size(); ++i) {
			IntersectionPoint spt;
			grp.computeShadingPoint(i, spt);
			Random& random = session.random(spt.Ray.PixelIndex);

			size_t occlusions = 0;
			for (size_t i = 0; i < mSampleCount; ++i) {
				const Vector2f rnd	= random.get2D();
				const Vector3f dir	= Sampling::hemi(rnd(0), rnd(1));
				const Vector3f ndir = Tangent::fromTangentSpace(spt.Surface.N, spt.Surface.Nx, spt.Surface.Ny,
																dir);

				const Ray n = spt.Ray.next(spt.P, ndir, spt.Surface.N, UsedRayType, PR_EPSILON, spt.Ray.MaxT);

				if (session.traceShadowRay(n))
					++occlusions;
			}

			const SpectralBlob weight = SpectralBlob(1.0f - occlusions / (float)mSampleCount);

			session.pushSPFragment(spt, stdPath);
			session.pushSpectralFragment(SpectralBlob::Ones(), SpectralBlob::Ones(), weight, spt.Ray, stdPath);
		}
	}

	void onTile(RenderTileSession& session) override
	{
		PR_PROFILE_THIS;

		while (!session.pipeline()->isFinished()) {
			session.pipeline()->runPipeline();
			while (session.pipeline()->hasShadingGroup()) {
				auto sg = session.pipeline()->popShadingGroup(session);
				if (sg.isBackground())
					session.tile()->statistics().add(RenderStatisticEntry::BackgroundHitCount, sg.size());
				else
					handleShadingGroup(session, sg);
			}
		}
	}

private:
	const size_t mSampleCount;
};

class IntAO : public IIntegrator {
public:
	explicit IntAO(size_t sample_count)
		: mSampleCount(sample_count)
	{
	}

	virtual ~IntAO() = default;

	std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext*, size_t) override
	{
		return std::make_shared<IntAOInstance>(mSampleCount);
	}

private:
	const size_t mSampleCount;
};

class IntAOFactory : public IIntegratorFactory {
public:
	explicit IntAOFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		size_t sample_count = (size_t)mParams.getUInt("sample_count", 10);
		return std::make_shared<IntAO>(sample_count);
	}

private:
	ParameterGroup mParams;
};

class IntAOFactoryFactory : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntAOFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "ao", "occlusion", "ambient_occlusion" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Ambient Occlusion", "Simple integrator to calculate ambient_occlusion")
			.Identifiers(getNames())
			.Inputs()
			.UInt("sample_count", "Samples used to determine ambient_occlusion", 10)
			.Specification()
			.get();
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntAOFactoryFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)