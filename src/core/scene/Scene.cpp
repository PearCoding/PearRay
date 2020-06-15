#include "Scene.h"
#include "Platform.h"
#include "camera/ICamera.h"
#include "emission/IEmission.h"
#include "infinitelight/IInfiniteLight.h"
#include "material/IMaterial.h"
#include "renderer/RenderContext.h"
#include "serialization/FileSerializer.h"

#include "Logger.h"

#include <boost/filesystem.hpp>
#include <fstream>

#define BUILDER kdTreeBuilder

namespace PR {
Scene::Scene(const std::shared_ptr<ICamera>& activeCamera,
			 const std::vector<std::shared_ptr<IEntity>>& entities,
			 const std::vector<std::shared_ptr<IMaterial>>& materials,
			 const std::vector<std::shared_ptr<IEmission>>& emissions,
			 const std::vector<std::shared_ptr<IInfiniteLight>>& infLights)
	: mActiveCamera(activeCamera)
	, mEntities(entities)
	, mMaterials(materials)
	, mEmissions(emissions)
	, mInfLights(infLights)
{
	PR_LOG(L_DEBUG) << "Setup before scene build..." << std::endl;
	mActiveCamera->beforeSceneBuild();
	for (auto o : mEmissions)
		o->beforeSceneBuild();
	for (auto o : mInfLights)
		o->beforeSceneBuild();
	for (auto o : mMaterials)
		o->beforeSceneBuild();
	for (auto o : mEntities)
		o->beforeSceneBuild();

	PR_LOG(L_DEBUG) << "Setup after scene build..." << std::endl;
	mActiveCamera->afterSceneBuild(this);
	for (auto o : mEmissions)
		o->afterSceneBuild(this);
	for (auto o : mInfLights)
		o->afterSceneBuild(this);
	for (auto o : mMaterials)
		o->afterSceneBuild(this);
	for (auto o : mEntities)
		o->afterSceneBuild(this);
}

Scene::~Scene()
{
}
void Scene::beforeRender(RenderContext* ctx)
{
	PR_LOG(L_DEBUG) << "Setup before render start..." << std::endl;

	mActiveCamera->beforeRender(ctx);
	for (auto o : mEmissions)
		o->beforeRender(ctx);
	for (auto o : mInfLights)
		o->beforeRender(ctx);
	for (auto o : mMaterials)
		o->beforeRender(ctx);
	for (auto o : mEntities)
		o->beforeRender(ctx);

	mDeltaInfLights.clear();
	mNonDeltaInfLights.clear();
	for (auto o : mInfLights) {
		if (o->hasDeltaDistribution())
			mDeltaInfLights.push_back(o);
		else
			mNonDeltaInfLights.push_back(o);
	}
}

void Scene::afterRender(RenderContext* ctx)
{
	PR_LOG(L_DEBUG) << "Setup after render stop..." << std::endl;

	mActiveCamera->afterRender(ctx);
	for (auto o : mEmissions)
		o->afterRender(ctx);
	for (auto o : mInfLights)
		o->afterRender(ctx);
	for (auto o : mMaterials)
		o->afterRender(ctx);
	for (auto o : mEntities)
		o->afterRender(ctx);
}

void Scene::traceRays(RayStream& rays, HitStream& hits) const
{
	// Split stream into specific groups
	hits.reset();
	while (rays.hasNextGroup()) {
		RayGroup grp = rays.getNextGroup();

		if (grp.isCoherent())
			traceCoherentRays(grp, hits);
		else
			traceIncoherentRays(grp, hits);
	}
}

void Scene::traceCoherentRays(const RayGroup& grp, HitStream& hits) const
{
	PR_PROFILE_THIS;

	// TODO
	PR_UNUSED(grp);
	PR_UNUSED(hits);
}

void Scene::traceIncoherentRays(const RayGroup& grp, HitStream& hits) const
{
	PR_PROFILE_THIS;

	// TODO
	PR_UNUSED(grp);
	PR_UNUSED(hits);
}
} // namespace PR
