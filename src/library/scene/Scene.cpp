#include "Scene.h"
#include "Platform.h"
#include "camera/ICamera.h"
#include "container/kdTreeBuilder.h"
#include "container/kdTreeBuilderNaive.h"
#include "emission/IEmission.h"
#include "infinitelight/IInfiniteLight.h"
#include "material/IMaterial.h"
#include "renderer/RenderContext.h"

#include "Logger.h"

#include <boost/filesystem.hpp>
#include <fstream>

#define BUILDER kdTreeBuilder

namespace PR {
Scene::Scene(const std::shared_ptr<ICamera>& activeCamera,
			 const std::vector<std::shared_ptr<IEntity>>& entities,
			 const std::vector<std::shared_ptr<IMaterial>>& materials,
			 const std::vector<std::shared_ptr<IEmission>>& emissions,
			 const std::vector<std::shared_ptr<IInfiniteLight>>& infLights,
			 const std::wstring& cntFile)
	: mActiveCamera(activeCamera)
	, mEntities(entities)
	, mMaterials(materials)
	, mEmissions(emissions)
	, mInfLights(infLights)
	, mKDTree(nullptr)
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

	PR_LOG(L_DEBUG) << "Starting to build global space-partitioning structure " << boost::filesystem::path(cntFile) << std::endl;
	buildTree(cntFile);
	loadTree(cntFile);

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

void Scene::buildTree(const std::wstring& file)
{
	PR_PROFILE_THIS;

	size_t count = mEntities.size();
	PR_LOG(L_INFO) << count << " Entities" << std::endl;

	BUILDER builder(this,
					[](void* data, size_t index) { return reinterpret_cast<Scene*>(data)->mEntities[index]->worldBoundingBox(); },
					[](void* data, size_t index) {
						return reinterpret_cast<Scene*>(data)->mEntities[index]->collisionCost();
					},
					[](void* data, size_t index, size_t id) {
						reinterpret_cast<Scene*>(data)->mEntities[index]->setContainerID(id);
					});
	builder.setCostElementWise(true);
	builder.build(count);

	std::ofstream stream(encodePath(file));
	builder.save(stream);
}

void Scene::loadTree(const std::wstring& file)
{
	PR_PROFILE_THIS;

	std::ifstream stream(encodePath(file));
	mKDTree.reset(new kdTreeCollider);
	mKDTree->load(stream);
	if (!mKDTree->isEmpty())
		mBoundingBox = mKDTree->boundingBox();
}
} // namespace PR
