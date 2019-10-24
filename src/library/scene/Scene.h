#pragma once

#include "container/kdTreeCollider.h"
#include "entity/IEntity.h"
#include "geometry/BoundingBox.h"
#include "math/Compression.h"
#include "ray/RayStream.h"
#include "trace/HitStream.h"
#include "trace/ShadowHit.h"

#include <string>
#include <vector>

namespace PR {
class ICamera;
class IEntity;
class IMaterial;
class IEmission;
class IInfiniteLight;
class RenderContext;

class RayStream;
class HitStream;

class PR_LIB Scene {
public:
	Scene(const std::shared_ptr<ICamera>& activeCamera,
		  const std::vector<std::shared_ptr<IEntity>>& entities,
		  const std::vector<std::shared_ptr<IMaterial>>& materials,
		  const std::vector<std::shared_ptr<IEmission>>& emissions,
		  const std::vector<std::shared_ptr<IInfiniteLight>>& infLights,
		  const std::string& cntFile);
	virtual ~Scene();

	const std::vector<std::shared_ptr<IEntity>>& entities() const { return mEntities; }
	const std::vector<std::shared_ptr<IMaterial>>& materials() const { return mMaterials; }
	const std::vector<std::shared_ptr<IEmission>>& emissions() const { return mEmissions; }
	const std::vector<std::shared_ptr<IInfiniteLight>>& infiniteLights() const { return mInfLights; }

	std::shared_ptr<ICamera> activeCamera() const { return mActiveCamera; }

	template <typename Func>
	inline void traceRays(RayStream& rays, HitStream& hits, Func nonHit) const;
	inline ShadowHit traceShadowRay(const Ray& ray) const;

	inline const BoundingBox& boundingBox() const { return mBoundingBox; }

private:
	template <typename Func>
	inline void traceCoherentRays(const RayGroup& grp, HitStream& hits, Func nonHit) const;
	template <typename Func>
	inline void traceIncoherentRays(const RayGroup& grp, HitStream& hits, Func nonHit) const;

	void buildTree(const std::string& file);
	void loadTree(const std::string& file);

	std::shared_ptr<ICamera> mActiveCamera;
	std::vector<std::shared_ptr<IEntity>> mEntities;
	std::vector<std::shared_ptr<IMaterial>> mMaterials;
	std::vector<std::shared_ptr<IEmission>> mEmissions;
	std::vector<std::shared_ptr<IInfiniteLight>> mInfLights;

	std::unique_ptr<class kdTreeCollider> mKDTree;
	BoundingBox mBoundingBox;
};
} // namespace PR

#include "Scene.inl"