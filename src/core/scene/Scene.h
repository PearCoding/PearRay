#pragma once

#include "Profiler.h"
#include "archive/IArchive.h"
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
class RenderContext;
class ServiceObserver;
class SceneDatabase;

class PR_LIB_CORE Scene : public IArchive {
public:
	Scene(const std::shared_ptr<ServiceObserver>& serviceObserver,
		  const std::shared_ptr<ICamera>& activeCamera,
		  const std::shared_ptr<SceneDatabase>& database);
	virtual ~Scene();

	inline std::shared_ptr<ICamera> activeCamera() const { return mActiveCamera; }
	inline std::shared_ptr<SceneDatabase> database() const { return mDatabase; }
	inline std::shared_ptr<ServiceObserver> serviceObserver() const { return mServiceObserver; }

	// Some information:
	size_t entityCount() const;
	size_t emissionCount() const;
	size_t materialCount() const;
	size_t infiniteLightCount() const;
	size_t nodeCount() const;

	/// Traces a stream of rays and produces a stream of hits
	void traceRays(RayStream& rays, HitStream& hits) const override;
	/// Traces a single ray and returns true if something was hit. Information about the hit are updated if a hit was found
	bool traceSingleRay(const Ray& ray, HitEntry& entry) const override;
	/// Traces a single ray and returns true if something lays within the given distance
	bool traceShadowRay(const Ray& ray, float distance = PR_INF) const override;

	inline virtual BoundingBox boundingBox() const override { return mBoundingBox; }
	inline const Sphere& boundingSphere() const { return mBoundingSphere; }

	void beforeRender(RenderContext* ctx);
	void afterRender(RenderContext* ctx);

private:
	void setupScene();

	const std::shared_ptr<ServiceObserver> mServiceObserver;

	std::shared_ptr<ICamera> mActiveCamera;
	std::shared_ptr<SceneDatabase> mDatabase;

	std::unique_ptr<struct SceneInternal> mInternal;
	BoundingBox mBoundingBox;
	Sphere mBoundingSphere;
};
} // namespace PR