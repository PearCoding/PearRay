#include "Scene.h"
#include "entity/Entity.h"
#include "ray/Ray.h"

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

	// Really bad implementation!
	Entity* Scene::checkCollision(const Ray& ray, PM::vec3& collisionPoint) const
	{
		Entity* res = nullptr;
		float n = -1;
		PM::vec3 tmpCollisionPoint;
		for (Entity* e : mEntities)
		{
			if (e->checkCollision(ray, tmpCollisionPoint))
			{
				float l = PM::pm_Magnitude3D(PM::pm_Subtract(tmpCollisionPoint, ray.startPosition()));

				if (n == -1 || l < n)
				{
					n = l;
					res = e;
					collisionPoint = tmpCollisionPoint;
				}
			}
		}

		return res;
	}
}