#pragma once

#include "Random.h"
#include "buffer/Feedback.h"
#include "buffer/OutputBuffer.h"
#include "entity/IEntity.h"
#include "geometry/GeometryPoint.h"
#include "ray/RayPackage.h"
#include "ray/RayStream.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileStatistics.h"
#include "scene/Scene.h"
#include "trace/HitStream.h"
#include "trace/ShadowHit.h"

namespace PR {

class RenderTile;
class IMaterial;
class IEmission;
class ShadingPoint;
class LightPath;

class PR_LIB RenderTileSession {
public:
	RenderTileSession(uint32 threadIndex, RenderTile* tile,
					  const std::shared_ptr<OutputBufferBucket>& bucket,
					  RayStream* rayStream,
					  HitStream* hitStream);
	~RenderTileSession();

	inline uint32 threadID() const
	{
		return mThread;
	}

	inline RenderTile* tile() const
	{
		return mTile;
	}

	inline HitStream* hitStream() const
	{
		return mHitStream;
	}

	inline RayStream* rayStream() const
	{
		return mRayStream;
	}

	IEntity* getEntity(uint32 id) const;
	IMaterial* getMaterial(uint32 id) const;
	IEmission* getEmission(uint32 id) const;

	bool handleCameraRays();

	inline void enqueueCameraRay(const Ray& ray);
	inline void enqueueLightRay(const Ray& ray);
	inline bool enoughRaySpace(size_t requested = 1) const;
	inline Ray getRay(size_t id) const;

	bool traceBounceRay(const Ray& ray, GeometryPoint& pt, IEntity*& entity, IMaterial*& material) const;
	ShadowHit traceShadowRay(const Ray& ray) const;

	inline size_t maxBufferCount() const;

	template <typename Func1, typename Func2>
	inline void handleHits(Func1 nonhitFunc, Func2 hitFunc);

	void pushFragment(const ShadingPoint& pt, const LightPath& path) const;
	void pushFeedbackFragment(const Ray& ray, uint32 feedback) const;

	IEntity* pickRandomLight(const Vector3f& view, GeometryPoint& pt, float& pdf) const;

private:
	void startShadingGroup(const ShadingGroup& grp, IEntity*& entity, IMaterial*& material);
	void endShadingGroup(const ShadingGroup& grp);

	std::pair<uint32, uint32> localCoordinates(uint32 pixelIndex) const;

	uint32 currentIndex() const;

	uint32 mThread;
	RenderTile* mTile;
	std::shared_ptr<OutputBufferBucket> mBucket;
	RayStream* mRayStream;
	HitStream* mHitStream;

	uint32 mCurrentX;
	uint32 mCurrentY;
};
} // namespace PR

#include "RenderTileSession.inl"