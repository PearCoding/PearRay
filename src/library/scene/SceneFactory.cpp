#include "SceneFactory.h"
#include "Scene.h"
#include "entity/Entity.h"
#include "entity/RenderEntity.h"
#include "light/IInfiniteLight.h"

#include "Logger.h"

namespace PR {

SceneFactory::SceneFactory(const std::string& name)
	: mName(name)
{
}

SceneFactory::~SceneFactory()
{
}

void SceneFactory::addEntity(const std::shared_ptr<Entity>& e)
{
	PR_ASSERT(e, "Given entity should be valid");
	if (e->isRenderable())
		mRenderEntities.push_back(std::static_pointer_cast<RenderEntity>(e));
	else
		mEntities.push_back(e);
}

void SceneFactory::removeEntity(const std::shared_ptr<Entity>& e)
{
	PR_ASSERT(e, "Given entity should be valid");
	if (e->isRenderable())
		mRenderEntities.remove(std::static_pointer_cast<RenderEntity>(e));
	else
		mEntities.remove(e);
}

std::shared_ptr<Entity> SceneFactory::getEntity(const std::string& name, const std::string& type) const
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

void SceneFactory::addInfiniteLight(const std::shared_ptr<IInfiniteLight>& e)
{
	PR_ASSERT(e, "Given light should be valid");
	mInfiniteLights.push_back(e);
}

void SceneFactory::removeInfiniteLight(const std::shared_ptr<IInfiniteLight>& e)
{
	PR_ASSERT(e, "Given light should be valid");
	mInfiniteLights.remove(e);
}

void SceneFactory::setActiveCamera(const std::shared_ptr<Camera>& c)
{
	PR_ASSERT(c, "Given camera should be valid");
	mActiveCamera = c;
}

const std::shared_ptr<Camera>& SceneFactory::activeCamera() const
{
	return mActiveCamera;
}

void SceneFactory::clear()
{
	mEntities.clear();
	mRenderEntities.clear();
	mInfiniteLights.clear();
	mActiveCamera.reset();
}

std::shared_ptr<Scene> SceneFactory::create() const
{
	return std::make_shared<Scene>(mName, mActiveCamera, mEntities, mRenderEntities, mInfiniteLights);
}
}
