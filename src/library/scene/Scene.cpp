#include "Scene.h"
#include "container/kdTree.h"
#include "entity/Entity.h"
#include "entity/RenderEntity.h"
#include "light/IInfiniteLight.h"
#include "material/Material.h"

#include "Logger.h"

namespace PR {
typedef kdTree<std::shared_ptr<RenderEntity>, Scene, true> SceneKDTree;

Scene::Scene(const std::string& name,
			 const std::shared_ptr<Camera>& activeCamera,
			 const std::list<std::shared_ptr<Entity>>& entities,
			 const std::list<std::shared_ptr<RenderEntity>>& renderentities,
			 const std::list<std::shared_ptr<IInfiniteLight>>& lights)
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
		delete reinterpret_cast<SceneKDTree*>(mKDTree);
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

void Scene::buildTree(bool force)
{
	if (mKDTree) {
		if (force) {
			PR_LOG(L_DEBUG) << "kdTree already exists, deleting old one." << std::endl;
			delete reinterpret_cast<SceneKDTree*>(mKDTree);
			mKDTree = nullptr;
		} else {
			return;
		}
	}

	PR_LOG(L_INFO) << mRenderEntities.size() << " Render Entities" << std::endl;

	mKDTree = new SceneKDTree(this,
							  [](Scene*, const std::shared_ptr<RenderEntity>& e) { return e->worldBoundingBox(); },
							  [](Scene*, const std::shared_ptr<RenderEntity>& e) {
								  return e->collisionCost();
							  },
							  [](Scene*, const std::shared_ptr<RenderEntity>& e, uint32 id) {
								  e->setContainerID(id);
							  });

	reinterpret_cast<SceneKDTree*>(mKDTree)->enableCostForLeafDetection(false);
	reinterpret_cast<SceneKDTree*>(mKDTree)->setMinimumObjectsForLeaf(1);

	reinterpret_cast<SceneKDTree*>(mKDTree)->build(
		mRenderEntities.begin(), mRenderEntities.end(), mRenderEntities.size());
}

SceneCollision Scene::checkCollision(const Ray& ray) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	SceneCollision sc;
	std::shared_ptr<RenderEntity> entity;
	sc.Successful = reinterpret_cast<SceneKDTree*>(
						mKDTree)
						->checkCollision(ray, entity, sc.Point,
										 [](Scene*, const Ray& ray, FacePoint& point, const std::shared_ptr<RenderEntity>& e) {
											 RenderEntity::Collision c = e->checkCollision(ray);
											 point					   = c.Point;
											 return (c.Successful
													 && ((ray.flags() & RF_Debug)
														 || ((ray.flags() & RF_Light) ? c.Point.Material->allowsShadow() : c.Point.Material->isCameraVisible())));
										 });
	sc.Entity = entity.get(); // We use direct pointer here!
	return sc;
}

SceneCollision Scene::checkCollisionBoundingBox(const Ray& ray) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	SceneCollision sc;
	std::shared_ptr<RenderEntity> entity;
	sc.Successful = reinterpret_cast<SceneKDTree*>(
						mKDTree)
						->checkCollision(ray, entity, sc.Point,
										 [](Scene*, const Ray& ray, FacePoint& point, const std::shared_ptr<RenderEntity>& e) {
											 BoundingBox bx = e->worldBoundingBox();

											 BoundingBox::Intersection in = bx.intersects(ray);

											 if (in.Successful) {
												 point.P = in.Position;
											 }

											 return in.Successful;
										 });
	sc.Entity = entity.get(); // We use direct pointer here!
	return sc;
}

SceneCollision Scene::checkCollisionSimple(const Ray& ray) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	SceneCollision sc;
	sc.Successful = reinterpret_cast<SceneKDTree*>(
						mKDTree)
						->checkCollisionSimple(ray, sc.Point,
											   [](Scene*, const Ray& ray, FacePoint& point, const std::shared_ptr<RenderEntity>& e) {
												   RenderEntity::Collision c = e->checkCollision(ray);
												   point					 = c.Point;
												   return (c.Successful
														   && ((ray.flags() & RF_Debug)
															   || ((ray.flags() & RF_Light) ? c.Point.Material->allowsShadow() : c.Point.Material->isCameraVisible())));
											   });
	sc.Entity = nullptr;
	return sc;
}

void Scene::setup(RenderContext* context, bool force)
{
	PR_LOG(L_INFO) << "Freezing scene" << std::endl;
	for (auto e : mEntities)
		e->freeze(context);

	for (auto e : mRenderEntities)
		e->freeze(context);

	for (auto e : mInfiniteLights)
		e->freeze(context);

	PR_LOG(L_INFO) << "Starting to build global space-partitioning structure" << std::endl;
	buildTree(force);
}

BoundingBox Scene::boundingBox() const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	return reinterpret_cast<SceneKDTree*>(mKDTree)->boundingBox();
}
} // namespace PR
