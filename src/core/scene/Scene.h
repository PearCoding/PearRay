#pragma once

#include "Profiler.h"
#include "entity/IEntity.h"
#include "geometry/BoundingBox.h"
#include "math/Compression.h"
#include "ray/RayStream.h"
#include "trace/HitStream.h"

#include <string>
#include <vector>

namespace PR {
class ICamera;
class IEntity;
class IMaterial;
class IEmission;
class IInfiniteLight;
class INode;
class LightSampler;
class RenderContext;

class RayStream;
class HitStream;

class PR_LIB_CORE Scene {
public:
	Scene(const std::shared_ptr<ICamera>& activeCamera,
		  const std::vector<std::shared_ptr<IEntity>>& entities,
		  const std::vector<std::shared_ptr<IMaterial>>& materials,
		  const std::vector<std::shared_ptr<IEmission>>& emissions,
		  const std::vector<std::shared_ptr<IInfiniteLight>>& infLights,
		  const std::vector<std::shared_ptr<INode>>& nodes);
	virtual ~Scene();

	const std::vector<std::shared_ptr<IEntity>>& entities() const { return mEntities; }
	const std::vector<std::shared_ptr<IMaterial>>& materials() const { return mMaterials; }
	const std::vector<std::shared_ptr<IEmission>>& emissions() const { return mEmissions; }
	const std::vector<std::shared_ptr<IInfiniteLight>>& infiniteLights() const { return mInfLights; }
	const std::vector<std::shared_ptr<IInfiniteLight>>& deltaInfiniteLights() const { return mDeltaInfLights; }
	const std::vector<std::shared_ptr<IInfiniteLight>>& nonDeltaInfiniteLights() const { return mNonDeltaInfLights; }
	const std::vector<std::shared_ptr<INode>>& nodes() const { return mNodes; }

	std::shared_ptr<ICamera> activeCamera() const { return mActiveCamera; }

	void traceRays(RayStream& rays, HitStream& hits) const;
	bool traceSingleRay(const Ray& ray, HitEntry& entry) const;
	bool traceShadowRay(const Ray& ray, float distance, uint32 entity_id) const;
	bool traceOcclusionRay(const Ray& ray) const;

	inline BoundingBox boundingBox() const { return mBoundingBox; }

	void beforeRender(RenderContext* ctx);
	void afterRender(RenderContext* ctx);

private:
	void setupScene();

	std::shared_ptr<ICamera> mActiveCamera;
	std::vector<std::shared_ptr<IEntity>> mEntities;
	std::vector<std::shared_ptr<IMaterial>> mMaterials;
	std::vector<std::shared_ptr<IEmission>> mEmissions;
	std::vector<std::shared_ptr<IInfiniteLight>> mInfLights;
	std::vector<std::shared_ptr<IInfiniteLight>> mDeltaInfLights;
	std::vector<std::shared_ptr<IInfiniteLight>> mNonDeltaInfLights;
	std::vector<std::shared_ptr<INode>> mNodes;

	std::shared_ptr<LightSampler> mLightSampler;

	std::unique_ptr<struct SceneInternal> mInternal;
	BoundingBox mBoundingBox;
};
} // namespace PR