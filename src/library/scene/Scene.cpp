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

	void Scene::addEntity(const std::shared_ptr<Entity>& e)
	{
		PR_ASSERT(e, "Given entity should be valid");
		if(e->isRenderable())
			mRenderEntities.push_back(std::static_pointer_cast<RenderEntity>(e));
		else
			mEntities.push_back(e);
	}

	void Scene::removeEntity(const std::shared_ptr<Entity>& e)
	{
		PR_ASSERT(e, "Given entity should be valid");
		if(e->isRenderable())
			mRenderEntities.remove(std::static_pointer_cast<RenderEntity>(e));
		else
			mEntities.remove(e);
	}

	std::shared_ptr<Entity> Scene::getEntity(const std::string& name, const std::string& type) const
	{
		for (const auto& entity : mEntities)
		{
			if (entity->name() == name && entity->type() == type)
				return entity;
		}

		for (const auto& entity : mRenderEntities)
		{
			if (entity->name() == name && entity->type() == type)
				return entity;
		}

		return nullptr;
	}

	void Scene::addInfiniteLight(const std::shared_ptr<IInfiniteLight>& e)
	{
		PR_ASSERT(e, "Given light should be valid");
		mInfiniteLights.push_back(e);
	}

	void Scene::removeInfiniteLight(const std::shared_ptr<IInfiniteLight>& e)
	{
		PR_ASSERT(e, "Given light should be valid");
		mInfiniteLights.remove(e);
	}

	void Scene::clear()
	{
		mEntities.clear();
		mRenderEntities.clear();
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

		std::vector<RenderEntity*> list;
		list.reserve(mRenderEntities.size());
		for(const auto& e : mRenderEntities)
			list.push_back(e.get());
		
		((SceneKDTree*)mKDTree)->build(list.begin(), list.end(), list.size(),
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
		for (const auto& e : mEntities)
			e->freeze();

		for (const auto& e : mInfiniteLights)
			e->freeze();
	}

	void Scene::setup(RenderContext* context)
	{
		for (const auto& e: mRenderEntities)
			e->setup(context);
	}

	BoundingBox Scene::boundingBox() const
	{
		PR_ASSERT(mKDTree, "kdTree has to be valid");
		return ((SceneKDTree*)mKDTree)->boundingBox();
	}
}
