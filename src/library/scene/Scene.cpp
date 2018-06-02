#include "Scene.h"
#include "container/kdTreeBuilder.h"
#include "container/kdTreeBuilderNaive.h"
#include "container/kdTreeCollider.h"
#include "entity/Entity.h"
#include "entity/RenderEntity.h"
#include "light/IInfiniteLight.h"
#include "material/Material.h"
#include "renderer/RenderContext.h"

#include "Logger.h"

#include <fstream>

#define BUILDER kdTreeBuilder

namespace PR {
Scene::Scene(const std::string& name,
			 const std::shared_ptr<Camera>& activeCamera,
			 const std::vector<std::shared_ptr<Entity>>& entities,
			 const std::vector<std::shared_ptr<RenderEntity>>& renderentities,
			 const std::vector<std::shared_ptr<IInfiniteLight>>& lights)
	: mName(name)
	, mActiveCamera(activeCamera)
	, mEntities(entities)
	, mRenderEntities(renderentities)
	, mInfiniteLights(lights)
	, mKDTree(nullptr)
{
}

Scene::~Scene()
{
	if (mKDTree) {
		delete mKDTree;
		mKDTree = nullptr;
	}
}

std::shared_ptr<Entity> Scene::getEntity(const std::string& name, const std::string& type) const
{
	for (auto entity : mEntities) {
		if (entity->name() == name && entity->type() == type)
			return entity;
	}

	for (auto entity : mRenderEntities) {
		if (entity->name() == name && entity->type() == type)
			return entity;
	}

	return nullptr;
}

std::shared_ptr<Camera> Scene::activeCamera() const
{
	return mActiveCamera;
}

void Scene::buildTree(const std::string& file)
{
	PR_LOG(L_INFO) << mRenderEntities.size() << " Render Entities" << std::endl;

	BUILDER builder(this,
					[](void* data, uint64 index) { return reinterpret_cast<Scene*>(data)->mRenderEntities[index]->worldBoundingBox(); },
					[](void* data, uint64 index) {
						return reinterpret_cast<Scene*>(data)->mRenderEntities[index]->collisionCost();
					},
					[](void* data, uint64 index, uint32 id) {
						reinterpret_cast<Scene*>(data)->mRenderEntities[index]->setContainerID(id);
					});
	builder.setCostElementWise(true);
	builder.build(mRenderEntities.size());

	std::ofstream stream(file);
	builder.save(stream);
}

void Scene::loadTree(const std::string& file)
{
	if (mKDTree) {
		delete mKDTree;
		mKDTree = nullptr;
	}

	std::ifstream stream(file);
	mKDTree = new kdTreeCollider;
	mKDTree->load(stream);
	if (!mKDTree->isEmpty())
		mBoundingBox = mKDTree->boundingBox();
}

SceneCollision Scene::checkCollision(const Ray& ray) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	SceneCollision sc;
	uint64 entity;
	sc.Successful = mKDTree
						->checkCollision(ray, entity, sc.Point,
										 [this](const Ray& ray, FacePoint& point, uint64 index, float& t) {
											 RenderEntity::Collision c = this->mRenderEntities[index]->checkCollision(ray);
											 point					   = c.Point;
											 t						   = (ray.origin() - c.Point.P).norm();
											 return (c.Successful
													 && ((ray.flags() & RF_Debug)
														 || ((ray.flags() & RF_Light) ? c.Point.Material->allowsShadow() : c.Point.Material->isCameraVisible())));
										 });
	if (sc.Successful)
		sc.Entity = mRenderEntities[entity].get(); // We use direct pointer here!
	else
		sc.Entity = nullptr;
	return sc;
}

SceneCollision Scene::checkCollisionBoundingBox(const Ray& ray) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	SceneCollision sc;
	uint64 entity;
	sc.Successful = mKDTree
						->checkCollision(ray, entity, sc.Point,
										 [this](const Ray& ray, FacePoint& point, uint64 index, float& t) {
											 BoundingBox bx = this->mRenderEntities[index]->worldBoundingBox();

											 BoundingBox::Intersection in = bx.intersects(ray);

											 if (in.Successful) {
												 point.P = in.Position;
											 }
											 t = (ray.origin() - point.P).norm();

											 return in.Successful;
										 });
	if (sc.Successful)
		sc.Entity = mRenderEntities[entity].get(); // We use direct pointer here!
	else
		sc.Entity = nullptr;
	return sc;
}

SceneCollision Scene::checkCollisionSimple(const Ray& ray) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	SceneCollision sc;
	sc.Successful = mKDTree
						->checkCollisionSimple(ray, sc.Point,
											   [this](const Ray& ray, FacePoint& point, uint64 index) {
												   RenderEntity::Collision c = this->mRenderEntities[index]->checkCollision(ray);
												   point					 = c.Point;
												   return (c.Successful
														   && ((ray.flags() & RF_Debug)
															   || ((ray.flags() & RF_Light) ? c.Point.Material->allowsShadow() : c.Point.Material->isCameraVisible())));
											   });
	sc.Entity = nullptr;
	return sc;
}

void Scene::setup(RenderContext* context)
{
	PR_LOG(L_INFO) << "Freezing scene" << std::endl;
	for (auto e : mEntities)
		e->freeze(context);

	for (auto e : mRenderEntities)
		e->freeze(context);

	for (auto e : mInfiniteLights)
		e->freeze(context);

	const std::string file = context->workingDir() + "scene.cnt";
	PR_LOG(L_INFO) << "Starting to build global space-partitioning structure \"" << file << "\"" << std::endl;
	buildTree(file);
	loadTree(file);
}
} // namespace PR
