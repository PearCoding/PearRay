#pragma once

#include "PR_Config.h"

#include <vector>
#include <string>

namespace PR {
class Camera;
class Entity;
class IInfiniteLight;
class RenderContext;
class RenderEntity;
class Scene;

class PR_LIB SceneFactory {
public:
	explicit SceneFactory(const std::string& name);
	virtual ~SceneFactory();

	inline void setName(const std::string& name) { mName = name; }
	inline std::string name() const { return mName; }

	void addEntity(const std::shared_ptr<Entity>& e);
	void removeEntity(const std::shared_ptr<Entity>& e);

	std::shared_ptr<Entity> getEntity(const std::string& name, const std::string& type) const;

	inline const std::vector<std::shared_ptr<RenderEntity>>& renderEntities() const { return mRenderEntities; }
	inline const std::vector<std::shared_ptr<Entity>>& entities() const { return mEntities; }

	void addInfiniteLight(const std::shared_ptr<IInfiniteLight>& e);
	void removeInfiniteLight(const std::shared_ptr<IInfiniteLight>& e);
	inline const std::vector<std::shared_ptr<IInfiniteLight>>& infiniteLights() const { return mInfiniteLights; }

	void setActiveCamera(const std::shared_ptr<Camera>& c);
	std::shared_ptr<Camera> activeCamera() const;

	void clear();

	std::shared_ptr<Scene> create() const;

	inline size_t fullEntityCount() const { return mEntities.size() + mRenderEntities.size(); }

private:
	std::string mName;
	std::shared_ptr<Camera> mActiveCamera;
	std::vector<std::shared_ptr<Entity>> mEntities;
	std::vector<std::shared_ptr<RenderEntity>> mRenderEntities;
	std::vector<std::shared_ptr<IInfiniteLight>> mInfiniteLights;
};
}
