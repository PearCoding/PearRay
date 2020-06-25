#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "integrator/IIntegratorPlugin.h"
#include "math/Projection.h"
#include "math/Tangent.h"

#include "path/LightPath.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "renderer/StreamPipeline.h"
#include "shader/ShadingPoint.h"

#include "Logger.h"

namespace PR {
constexpr RayFlags UsedRayType = RF_Bounce; // Do not use RF_Shadow, as most lights will be visible. We don't want that
class IntAOInstance : public IIntegratorInstance {
public:
	explicit IntAOInstance(RenderContext* ctx, size_t sample_count)
		: mPipeline(ctx)
		, mSampleCount(sample_count)
	{
	}

	virtual ~IntAOInstance() = default;

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& grp)
	{
		Random& random	  = session.tile()->random();
		LightPath stdPath = LightPath::createCDL(1);

		session.tile()->statistics().addEntityHitCount(grp.size());
		session.tile()->statistics().addDepthCount(grp.size());
		for (size_t i = 0; i < grp.size(); ++i) {
			ShadingPoint spt;
			grp.computeShadingPoint(i, spt);

			size_t occlusions = 0;
			float pdf;
			for (size_t i = 0; i < mSampleCount; ++i) {
				Vector2f rnd  = random.get2D();
				Vector3f dir  = Projection::hemi(rnd(0), rnd(1), pdf);
				Vector3f ndir = Tangent::fromTangentSpace(spt.Surface.N, spt.Surface.Nx, spt.Surface.Ny,
														  dir);

				const Ray n = spt.Ray.next(spt.P, ndir, spt.Surface.N, UsedRayType, PR_EPSILON, spt.Ray.MaxT);

				if (session.traceOcclusionRay(n))
					++occlusions;
			}

			SpectralBlob radiance = SpectralBlob::Ones();
			radiance *= 1.0f - occlusions / (float)mSampleCount;

			session.pushSPFragment(spt, stdPath);
			session.pushSpectralFragment(radiance, spt.Ray, stdPath);
		}
	}

	void onTile(RenderTileSession& session) override
	{
		PR_PROFILE_THIS;

		mPipeline.reset(session.tile());

		while (!mPipeline.isFinished()) {
			mPipeline.runPipeline();
			while (mPipeline.hasShadingGroup()) {
				auto sg = mPipeline.popShadingGroup(session);
				if (sg.isBackground())
					session.tile()->statistics().addBackgroundHitCount(sg.size());
				else
					handleShadingGroup(session, sg);
			}
		}
	}

private:
	StreamPipeline mPipeline;
	const size_t mSampleCount;
};

class IntAO : public IIntegrator {
public:
	explicit IntAO(size_t sample_count)
		: mSampleCount(sample_count)
	{
	}

	virtual ~IntAO() = default;

	std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext* ctx, size_t) override
	{
		return std::make_shared<IntAOInstance>(ctx, mSampleCount);
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
	std::shared_ptr<IIntegratorFactory> create(uint32, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntAOFactory>(ctx.Parameters);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "ao", "occlusion", "ambient_occlusion" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntAOFactoryFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)