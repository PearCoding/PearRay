#pragma once

#include "geometry/BoundingBox.h"
#include "shader/FacePoint.h"

#include <list>
#include <string>

namespace PR {
class Camera;
class Entity;
struct FacePoint;
class IInfiniteLight;
class RenderContext;
class RenderEntity;
class Ray;

struct SceneCollision {
	bool Successful;
	RenderEntity* Entity;
	FacePoint Point;
};

class PR_LIB Scene {
public:
	Scene(const std::string& name,
		  const std::shared_ptr<Camera>& activeCamera,
		  const std::list<std::shared_ptr<Entity>>& entities,
		  const std::list<std::shared_ptr<RenderEntity>>& renderentities,
		  const std::list<std::shared_ptr<IInfiniteLight>>& lights);
	virtual ~Scene();

	inline const std::string& name() const { return mName; }

	std::shared_ptr<Entity> getEntity(const std::string& name, const std::string& type) const;

	inline const std::list<std::shared_ptr<RenderEntity>>& renderEntities() const { return mRenderEntities; }
	inline const std::list<std::shared_ptr<Entity>>& entities() const { return mEntities; }

	inline const std::list<std::shared_ptr<IInfiniteLight>>& infiniteLights() const { return mInfiniteLights; }

	std::shared_ptr<Camera> activeCamera() const;

	SceneCollision checkCollision(const Ray& ray) const;
	SceneCollision checkCollisionSimple(const Ray& ray) const;

	void setup(RenderContext* context, bool force = false);

	BoundingBox boundingBox() const;

private:
	void buildTree(bool force);
	void freeze();

	const std::string mName;
	const std::shared_ptr<Camera> mActiveCamera;
	const std::list<std::shared_ptr<Entity>> mEntities;
	const std::list<std::shared_ptr<RenderEntity>> mRenderEntities;
	const std::list<std::shared_ptr<IInfiniteLight>> mInfiniteLights;

	void* mKDTree; // We use a void* pointer to hide the KDTree header only implementation
};
}
