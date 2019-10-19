#pragma once

#include "Random.h"
#include "buffer/Feedback.h"
#include "buffer/OutputBuffer.h"
#include "entity/IEntity.h"
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
					  RayStream* rayStream,
					  HitStream* hitStream);
	~RenderTileSession();

	inline uint32 thread() const
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

	IEntity* getEntity(uint32 id) const;
	IMaterial* getMaterial(uint32 id) const;
	IEmission* getEmission(uint32 id) const;

	bool handleCameraRays();

	inline void enqueueRay(const Ray& ray);
	inline bool enoughRaySpace(size_t requested = 1) const;
	inline void sendRay(size_t id, const Ray& ray);
	inline Ray getRay(size_t id) const;

	ShadowHit traceShadowRay(const Ray& ray) const;

	inline size_t maxBufferCount() const;

	template <typename Func>
	inline void handleHits(Func hitFunc);

	void pushFragment(const ShadingPoint& pt, const LightPath& path) const;
	void pushNonHitFragment(const ShadingPoint& pt) const;
	void pushFeedbackFragment(const Ray& ray, uint32 feedback) const;

	IEntity* pickRandomLight(GeometryPoint& pt, float& pdf) const;

private:
	void startShadingGroup(const ShadingGroup& grp, IEntity*& entity, IMaterial*& material);
	void endShadingGroup(const ShadingGroup& grp);

	uint32 currentIndex() const;

	uint32 mThread;
	RenderTile* mTile;
	RayStream* mRayStream;
	HitStream* mHitStream;

	uint32 mCurrentX;
	uint32 mCurrentY;
};
} // namespace PR

#include "RenderTileSession.inl"