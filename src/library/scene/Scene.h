#pragma once

#include "geometry/BoundingBox.h"
#include "shader/FacePoint.h"

#include <vector>
#include <string>

namespace PR {
class Camera;
class Entity;
struct FacePoint;
class IInfiniteLight;
class RenderContext;
class RenderEntity;
class Ray;
class Registry;

struct SceneCollision {
	bool Successful;
	RenderEntity* Entity;
	FacePoint Point;
};

class PR_LIB Scene {
public:
	Scene(const std::string& name,
		  const std::shared_ptr<Camera>& activeCamera,
		  const std::vector<std::shared_ptr<Entity>>& entities,
		  const std::vector<std::shared_ptr<RenderEntity>>& renderentities,
		  const std::vector<std::shared_ptr<IInfiniteLight>>& lights);
	virtual ~Scene();

	inline const std::string& name() const { return mName; }

	std::shared_ptr<Entity> getEntity(const std::string& name, const std::string& type) const;

	inline const std::vector<std::shared_ptr<RenderEntity>>& renderEntities() const { return mRenderEntities; }
	inline const std::vector<std::shared_ptr<Entity>>& entities() const { return mEntities; }

	inline const std::vector<std::shared_ptr<IInfiniteLight>>& infiniteLights() const { return mInfiniteLights; }

	std::shared_ptr<Camera> activeCamera() const;

	SceneCollision checkCollision(const Ray& ray) const;
	/* Simple collision detection. Only position is populated! */
	SceneCollision checkCollisionBoundingBox(const Ray& ray) const;
	SceneCollision checkCollisionSimple(const Ray& ray) const;

	void setup(RenderContext* context);

	inline const BoundingBox& boundingBox() const { return mBoundingBox; }

private:
	void buildTree(const std::string& file);
	void loadTree(const std::string& file);

	const std::string mName;
	const std::shared_ptr<Camera> mActiveCamera;
	const std::vector<std::shared_ptr<Entity>> mEntities;
	const std::vector<std::shared_ptr<RenderEntity>> mRenderEntities;
	const std::vector<std::shared_ptr<IInfiniteLight>> mInfiniteLights;

	class kdTreeCollider* mKDTree;
	BoundingBox mBoundingBox;
};
} // namespace PR
