#include "Scene.h"
#include "entity/Entity.h"

namespace PR
{
	Scene::Scene(const std::string& name) :
		mName(name)
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
	}
}