#include "Scene.h"
#include "kdTree.h"
#include "entity/Entity.h"
#include "entity/RenderEntity.h"

#include "Logger.h"

namespace PR
{
	Scene::Scene(const std::string& name) :
		mName(name), mKDTree(nullptr)
	{
	}

	Scene::~Scene()
	{
		clear();
	}

	void Scene::setName(const std::string& name)
	{
		mName = name;
	}

	std::string Scene::name() const
	{
		return mName;
	}

	void Scene::addEntity(Entity* e)
	{
		PR_ASSERT(e);
		mEntities.push_back(e);
		if (e->isRenderable())
		{
			mRenderEntities.push_back((RenderEntity*)e);
		}
	}

	void Scene::removeEntity(Entity* e)
	{
		PR_ASSERT(e);
		mEntities.remove(e);

		if (e->isRenderable())
		{
			mRenderEntities.remove((RenderEntity*)e);
		}
	}

	Entity* Scene::getEntity(const std::string& name, const std::string& type) const
	{
		for (Entity* entity : mEntities)
		{
			if (entity->name() == name && entity->type() == type)
			{
				return entity;
			}
		}

		return nullptr;
	}

	const std::list<Entity*>& Scene::entities() const
	{
		return mEntities;
	}

	const std::list<RenderEntity*>& Scene::renderEntities() const
	{
		return mRenderEntities;
	}

	void Scene::clear()
	{
		for (Entity* e : mEntities)
		{
			delete e;
		}
		mEntities.clear();
		mRenderEntities.clear();

		if (mKDTree)
		{
			delete mKDTree;
			mKDTree = nullptr;
		}
	}

	void Scene::buildTree()
	{
		if (mKDTree)
		{
			PR_LOGGER.log(L_Info, M_Scene, "kdTree already exists, deleting old one.");
			delete mKDTree;
			mKDTree = nullptr;
		}

		mKDTree = new kdTree;
		mKDTree->build(mRenderEntities);
	}

	RenderEntity* Scene::checkCollision(const Ray& ray, FacePoint& collisionPoint, Entity* ignore) const
	{
		PR_ASSERT(mKDTree);
		return mKDTree->checkCollision(ray, collisionPoint, ignore);
	}
}