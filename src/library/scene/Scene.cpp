#include "Scene.h"
#include "container/kdTree.h"
#include "entity/Entity.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "light/IInfiniteLight.h"

#include "Logger.h"

namespace PR
{
	typedef kdTree<RenderEntity, true> SceneKDTree;

	Scene::Scene(const std::string& name) :
		mName(name), mKDTree(nullptr)
	{
	}

	Scene::~Scene()
	{
		clear();
	}

	void Scene::addEntity(Entity* e)
	{
		PR_ASSERT(e, "Given entity should be valid");
		mEntities.push_back(e);
		if (e->isRenderable())
			mRenderEntities.push_back((RenderEntity*)e);
	}

	void Scene::removeEntity(Entity* e)
	{
		PR_ASSERT(e, "Given entity should be valid");
		mEntities.remove(e);

		if (e->isRenderable())
			mRenderEntities.remove((RenderEntity*)e);
	}

	Entity* Scene::getEntity(const std::string& name, const std::string& type) const
	{
		for (Entity* entity : mEntities)
		{
			if (entity->name() == name && entity->type() == type)
				return entity;
		}

		return nullptr;
	}

	void Scene::addInfiniteLight(IInfiniteLight* e)
	{
		PR_ASSERT(e, "Given light should be valid");
		mInfiniteLights.push_back(e);
	}

	void Scene::removeInfiniteLight(IInfiniteLight* e)
	{
		PR_ASSERT(e, "Given light should be valid");
		mInfiniteLights.remove(e);
	}

	void Scene::clear()
	{
		for (Entity* e : mEntities)
			delete e;
		mEntities.clear();
		mRenderEntities.clear();

		for (IInfiniteLight* e : mInfiniteLights)
			delete e;
		mInfiniteLights.clear();

		if (mKDTree)
		{
			delete (SceneKDTree*)mKDTree;
			mKDTree = nullptr;
		}
	}

	void Scene::buildTree()
	{
		if (mKDTree)
		{
			PR_LOGGER.log(L_Info, M_Scene, "kdTree already exists, deleting old one.");
			delete (SceneKDTree*)mKDTree;
			mKDTree = nullptr;
		}

		PR_LOGGER.logf(L_Info, M_Scene, "%i Render Entities", mRenderEntities.size());

		mKDTree = new SceneKDTree(
			[](RenderEntity* e) {return e->worldBoundingBox();},
			[](const Ray& ray, FaceSample& point, RenderEntity* e) {
				return (e->checkCollision(ray, point) &&
					((ray.flags() & RF_Debug) ||
						((ray.flags() & RF_Light) ?
						point.Material->allowsShadow() :
						point.Material->isCameraVisible()
					)));
		},
		[](RenderEntity* e) {
			return (float)e->collisionCost();
		});
		((SceneKDTree*)mKDTree)->build(mRenderEntities.begin(), mRenderEntities.end(), mRenderEntities.size(),
			[](RenderEntity* e){ return !e->isCollidable(); });
	}

	RenderEntity* Scene::checkCollision(const Ray& ray, FaceSample& collisionPoint) const
	{
		PR_ASSERT(mKDTree, "kdTree has to be valid");
		return ((SceneKDTree*)mKDTree)->checkCollision(ray, collisionPoint);
	}

	bool Scene::checkIfCollides(const Ray& ray, FaceSample& collisionPoint) const
	{
		PR_ASSERT(mKDTree, "kdTree has to be valid");
		return ((SceneKDTree*)mKDTree)->checkIfCollides(ray, collisionPoint);
	}

	void Scene::freeze()
	{
		for (Entity* e : mEntities)
			e->freeze();

		for (IInfiniteLight* e : mInfiniteLights)
			e->freeze();
	}

	void Scene::setup(RenderContext* context)
	{
		for (RenderEntity* e: mRenderEntities)
			e->setup(context);
	}

	BoundingBox Scene::boundingBox() const
	{
		PR_ASSERT(mKDTree, "kdTree has to be valid");
		return ((SceneKDTree*)mKDTree)->boundingBox();
	}
}
