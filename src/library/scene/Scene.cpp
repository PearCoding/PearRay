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

#include <fstream>
#include <boost/filesystem.hpp>

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
	PR_LOG(L_INFO) << "Starting to build global space-partitioning structure \"" << boost::filesystem::path(cntFile) << "\"" << std::endl;
	buildTree(cntFile);
	loadTree(cntFile);
}

Scene::~Scene()
{
}

void Scene::buildTree(const std::wstring& file)
{
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
	std::ifstream stream(encodePath(file));
	mKDTree.reset(new kdTreeCollider);
	mKDTree->load(stream);
	if (!mKDTree->isEmpty())
		mBoundingBox = mKDTree->boundingBox();
}
} // namespace PR
