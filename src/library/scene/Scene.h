#pragma once

#include "geometry/BoundingBox.h"

#include <string>
#include <vector>

namespace PR {
class ICamera;
class IEntity;
class RenderContext;
class RenderManager;
class Registry;

class RayStream;
class HitStream;

class PR_LIB Scene {
public:
	Scene(const RenderManager* manager);
	virtual ~Scene();

	std::shared_ptr<IEntity> getEntity(const std::string& name, const std::string& type) const;
	const std::vector<std::shared_ptr<IEntity>>& entities() const;

	inline const RenderManager* renderManager() const { return mRenderManager; }
	std::shared_ptr<ICamera> activeCamera() const { return mActiveCamera; }

	void traceCoherentRays(RayStream& rays, HitStream& hits) const;
	void traceIncoherentRays(RayStream& rays, HitStream& hits) const;
	void traceShadowRays(RayStream& rays, HitStream& hits) const;
	void setup(RenderContext* context);

	inline const BoundingBox& boundingBox() const { return mBoundingBox; }

private:
	void buildTree(const std::string& file);
	void loadTree(const std::string& file);

	const RenderManager* mRenderManager;
	const std::shared_ptr<ICamera> mActiveCamera;

	class kdTreeCollider* mKDTree;
	BoundingBox mBoundingBox;
};
} // namespace PR
