#include "Scene.h"
#include "camera/ICamera.h"
#include "container/kdTreeBuilder.h"
#include "container/kdTreeBuilderNaive.h"
#include "emission/IEmission.h"
#include "infinitelight/IInfiniteLight.h"
#include "material/IMaterial.h"
#include "renderer/RenderContext.h"

#include "Logger.h"

#include <fstream>

#define BUILDER kdTreeBuilder

namespace PR {
Scene::Scene(const std::shared_ptr<ICamera>& activeCamera,
			 const std::vector<std::shared_ptr<IEntity>>& entities,
			 const std::vector<std::shared_ptr<IMaterial>>& materials,
			 const std::vector<std::shared_ptr<IEmission>>& emissions,
			 const std::string& cntFile)
	: mActiveCamera(activeCamera)
	, mEntities(entities)
	, mMaterials(materials)
	, mEmissions(emissions)
	, mKDTree(nullptr)
{
	PR_LOG(L_INFO) << "Starting to build global space-partitioning structure \"" << cntFile << "\"" << std::endl;
	buildTree(cntFile);
	loadTree(cntFile);
}

Scene::~Scene()
{
}

void Scene::buildTree(const std::string& file)
{
	size_t count = mEntities.size();
	PR_LOG(L_INFO) << count << " Entities" << std::endl;

	BUILDER builder(this,
					[](void* data, uint64 index) { return reinterpret_cast<Scene*>(data)->mEntities[index]->worldBoundingBox(); },
					[](void* data, uint64 index) {
						return reinterpret_cast<Scene*>(data)->mEntities[index]->collisionCost();
					},
					[](void* data, uint64 index, uint32 id) {
						reinterpret_cast<Scene*>(data)->mEntities[index]->setContainerID(id);
					});
	builder.setCostElementWise(true);
	builder.build(count);

	std::ofstream stream(file);
	builder.save(stream);
}

void Scene::loadTree(const std::string& file)
{
	std::ifstream stream(file);
	mKDTree.reset(new kdTreeCollider);
	mKDTree->load(stream);
	if (!mKDTree->isEmpty())
		mBoundingBox = mKDTree->boundingBox();
}
} // namespace PR
