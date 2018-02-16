#include "Scene.h"
#include "container/kdTree.h"
#include "entity/Entity.h"
#include "entity/RenderEntity.h"
#include "light/IInfiniteLight.h"
#include "material/Material.h"

#include "Logger.h"

namespace PR {
typedef kdTree<RenderEntity, true> SceneKDTree;

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
	for (const auto& entity : mEntities) {
		if (entity->name() == name && entity->type() == type)
			return entity;
	}

	for (const auto& entity : mRenderEntities) {
		if (entity->name() == name && entity->type() == type)
			return entity;
	}

	return nullptr;
}

const std::shared_ptr<Camera>& Scene::activeCamera() const
{
	return mActiveCamera;
}

void Scene::buildTree(bool force)
{
	if (mKDTree) {
		if(force) {
			PR_LOGGER.log(L_Info, M_Scene, "kdTree already exists, deleting old one.");
			delete reinterpret_cast<SceneKDTree*>(mKDTree);
			mKDTree = nullptr;
		} else {
			return;
		}
	}

	PR_LOGGER.logf(L_Info, M_Scene, "%i Render Entities", mRenderEntities.size());

	mKDTree = new SceneKDTree(
		[](RenderEntity* e) { return e->worldBoundingBox(); },
		[](const Ray& ray, FacePoint& point, RenderEntity* e) {
			RenderEntity::Collision c = e->checkCollision(ray);
			point					  = c.Point;
			return (c.Successful
					&& ((ray.flags() & RF_Debug)
						|| ((ray.flags() & RF_Light) ? c.Point.Material->allowsShadow() : c.Point.Material->isCameraVisible())));
		},
		[](RenderEntity* e) {
			return e->collisionCost();
		},
		[](RenderEntity* e, uint32 id) {
			e->setContainerID(id);
		});

	std::vector<RenderEntity*> list;
	list.reserve(mRenderEntities.size());
	for (const auto& e : mRenderEntities)
		list.push_back(e.get());

	reinterpret_cast<SceneKDTree*>(mKDTree)->build(list.begin(), list.end(), list.size(), [](RenderEntity* e) { return !e->isCollidable(); });
}

SceneCollision Scene::checkCollision(const Ray& ray) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	SceneCollision sc;
	sc.Entity	 = reinterpret_cast<SceneKDTree*>(mKDTree)->checkCollision(ray, sc.Point);
	sc.Successful = (sc.Entity != nullptr);
	return sc;
}

SceneCollision Scene::checkCollisionSimple(const Ray& ray) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	SceneCollision sc;
	sc.Successful = reinterpret_cast<SceneKDTree*>(mKDTree)->checkCollisionSimple(ray, sc.Point);
	sc.Entity	 = nullptr;
	return sc;
}

void Scene::freeze()
{
	for (const auto& e : mEntities)
		e->freeze();

	for (const auto& e : mRenderEntities)
		e->freeze();

	for (const auto& e : mInfiniteLights)
		e->freeze();
}

void Scene::setup(RenderContext* context, bool force)
{
	PR_LOGGER.log(L_Info, M_Scene, "Freezing scene");
	freeze();
	PR_LOGGER.log(L_Info, M_Scene, "Starting to build global space-partitioning structure");
	buildTree(force);
	PR_LOGGER.log(L_Info, M_Scene, "Initializing render entities");
	for (const auto& e : mRenderEntities)
		e->setup(context);
}

BoundingBox Scene::boundingBox() const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	return reinterpret_cast<SceneKDTree*>(mKDTree)->boundingBox();
}
}
