#include "Scene.h"
#include "kdTree.h"
#include "entity/Entity.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"

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

		mKDTree = new SceneKDTree([](RenderEntity* e) {return e->worldBoundingBox();},
			[](const Ray& ray, SamplePoint& point, float& t, RenderEntity* e, RenderEntity* ignore) {
				if((!ignore || !e->isParent(ignore)) &&
				e->checkCollision(ray, point) &&
				point.Material && !point.Material->shouldIgnore(ray, point)) {
					t = PM::pm_MagnitudeSqr3D(PM::pm_Subtract(point.P, ray.startPosition()));
					return true;
				}
				else
					return false;
		},
		[](RenderEntity* e) {
			return (float)e->collisionCost();
		});
		((SceneKDTree*)mKDTree)->build(mRenderEntities);
	}

	RenderEntity* Scene::checkCollision(const Ray& ray, SamplePoint& collisionPoint, RenderEntity* ignore) const
	{
		float t;
		PR_ASSERT(mKDTree);
		return ((SceneKDTree*)mKDTree)->checkCollision(ray, collisionPoint, t, ignore);
	}

	void Scene::onPreRender()
	{
		for (Entity* e : mEntities)
		{
			e->onPreRender();
		}
	}
}
