#include "Scene.h"
#include "kdTree.h"
#include "entity/Entity.h"
#include "entity/GeometryEntity.h"

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

	void Scene::addEntity(GeometryEntity* e)
	{
		PR_ASSERT(e);
		mEntities.push_back(e);
		mGeometryEntities.push_back(e);
	}

	void Scene::removeEntity(GeometryEntity* e)
	{
		PR_ASSERT(e);
		mEntities.remove(e);
		mGeometryEntities.remove(e);
	}

	void Scene::addEntity(Entity* e)
	{
		PR_ASSERT(e);
		mEntities.push_back(e);
	}

	void Scene::removeEntity(Entity* e)
	{
		PR_ASSERT(e);
		mEntities.remove(e);
	}

	void Scene::clear()
	{
		for (Entity* e : mEntities)
		{
			delete e;
		}
		mEntities.clear();
		mGeometryEntities.clear();

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
		mKDTree->build(mGeometryEntities);
	}

	GeometryEntity* Scene::checkCollision(const Ray& ray, FacePoint& collisionPoint) const
	{
		PR_ASSERT(mKDTree);
		return mKDTree->checkCollision(ray, collisionPoint);
	}
}