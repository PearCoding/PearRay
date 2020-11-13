#pragma once

#include "Profiler.h"
#include "entity/IEntity.h"
#include "geometry/BoundingBox.h"
#include "geometry/Sphere.h"
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
class RenderContext;
class ServiceObserver;

class RayStream;
class HitStream;

class PR_LIB_CORE Scene {
public:
	Scene(const std::shared_ptr<ServiceObserver>& serviceObserver,
		  const std::shared_ptr<ICamera>& activeCamera,
		  const std::vector<std::shared_ptr<IEntity>>& entities,
		  const std::vector<std::shared_ptr<IMaterial>>& materials,
		  const std::vector<std::shared_ptr<IEmission>>& emissions,
		  const std::vector<std::shared_ptr<IInfiniteLight>>& infLights,
		  const std::vector<std::shared_ptr<INode>>& nodes);
	virtual ~Scene();

	inline const std::vector<std::shared_ptr<IEntity>>& entities() const { return mEntities; }
	inline const std::vector<std::shared_ptr<IMaterial>>& materials() const { return mMaterials; }
	inline const std::vector<std::shared_ptr<IEmission>>& emissions() const { return mEmissions; }
	inline const std::vector<std::shared_ptr<IInfiniteLight>>& infiniteLights() const { return mInfLights; }
	inline const std::vector<std::shared_ptr<IInfiniteLight>>& deltaInfiniteLights() const { return mDeltaInfLights; }
	inline const std::vector<std::shared_ptr<IInfiniteLight>>& nonDeltaInfiniteLights() const { return mNonDeltaInfLights; }
	inline const std::vector<std::shared_ptr<INode>>& nodes() const { return mNodes; }

	inline std::shared_ptr<ICamera> activeCamera() const { return mActiveCamera; }

	inline std::shared_ptr<ServiceObserver> serviceObserver() const { return mServiceObserver; }

	/// Traces a stream of rays and produces a stream of hits
	void traceRays(RayStream& rays, HitStream& hits) const;
	/// Traces a single ray and returns true if something was hit. Information about the hit are updated if a hit was found
	bool traceSingleRay(const Ray& ray, HitEntry& entry) const;
	/// Traces a single ray and returns true if something lays within the given distance
	bool traceShadowRay(const Ray& ray, float distance = PR_INF) const;

	inline const BoundingBox& boundingBox() const { return mBoundingBox; }
	inline const Sphere& boundingSphere() const { return mBoundingSphere; }

	void beforeRender(RenderContext* ctx);
	void afterRender(RenderContext* ctx);

private:
	void setupScene();

	const std::shared_ptr<ServiceObserver> mServiceObserver;

	std::shared_ptr<ICamera> mActiveCamera;
	std::vector<std::shared_ptr<IEntity>> mEntities;
	std::vector<std::shared_ptr<IMaterial>> mMaterials;
	std::vector<std::shared_ptr<IEmission>> mEmissions;
	std::vector<std::shared_ptr<IInfiniteLight>> mInfLights;
	std::vector<std::shared_ptr<IInfiniteLight>> mDeltaInfLights;
	std::vector<std::shared_ptr<IInfiniteLight>> mNonDeltaInfLights;
	std::vector<std::shared_ptr<INode>> mNodes;

	std::unique_ptr<struct SceneInternal> mInternal;
	BoundingBox mBoundingBox;
	Sphere mBoundingSphere;
};
} // namespace PR