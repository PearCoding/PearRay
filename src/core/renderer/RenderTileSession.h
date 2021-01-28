#pragma once

#include "Random.h"
#include "output/Feedback.h"
#include "entity/IEntity.h"
#include "geometry/GeometryPoint.h"
#include "ray/Ray.h"
#include "ray/RayStream.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderStatistics.h"
#include "scene/Scene.h"
#include "trace/HitStream.h"

namespace PR {

class IEmission;
class IMaterial;
class IntersectionPoint;
class LightPath;
class LocalOutputQueue;
class LocalOutputSystem;
class RayGroup;
class RenderTile;
class StreamPipeline;

class PR_LIB_CORE RenderTileSession {
public:
	RenderTileSession(); // Dummy session!
	RenderTileSession(uint32 threadIndex, RenderTile* tile, StreamPipeline* pipeline,
					  const std::shared_ptr<LocalOutputQueue>& localQueue,
					  const std::shared_ptr<LocalOutputSystem>& localSystem);
	~RenderTileSession();

	inline uint32 threadID() const { return mThread; }

	inline RenderTile* tile() const { return mTile; }
	inline const RenderContext* context() const { return mTile->context(); }
	inline Random& random(RandomSlot slot) { return mTile->random(slot); }
	inline Random& random(const Point2i& globalP) { return mTile->random(globalP); }
	inline Random& random(const Point1i& rayLocalPixelIndex) { return random(globalCoordinates(rayLocalPixelIndex)); }
	inline StreamPipeline* pipeline() const { return mPipeline; }

	IEntity* getEntity(uint32 id) const;
	IMaterial* getMaterial(uint32 id) const;
	IEmission* getEmission(uint32 id) const;

	inline const RayGroup& getRayGroup(const Ray& ray) const { return getRayGroup(ray.GroupID); }
	const RayGroup& getRayGroup(uint32 id) const;

	bool traceSingleRay(const Ray& ray, Vector3f& pos, GeometryPoint& pt, IEntity*& entity, IMaterial*& material) const;
	bool traceShadowRay(const Ray& ray, float distance = PR_INF) const;

	void pushSpectralFragment(float mis, const SpectralBlob& importance, const SpectralBlob& radiance,
							  const Ray& ray, const LightPath& path) const;
	void pushSPFragment(const IntersectionPoint& pt, const LightPath& path) const;
	void pushFeedbackFragment(uint32 feedback, const Ray& ray) const;

	void pushCustomSpectralFragment(uint32 queueID, const Ray& ray, const SpectralBlob& value);
	void pushCustom3DFragment(uint32 queueID, const Ray& ray, const Vector3f& value);
	void pushCustom1DFragment(uint32 queueID, const Ray& ray, float value);
	void pushCustomCounterFragment(uint32 queueID, const Ray& ray, uint32 value);

private:
	Point2i globalCoordinates(Point1i pixelIndex) const;
	Point2i localCoordinates(Point1i pixelIndex) const;

	uint32 currentIndex() const;

	uint32 mThread;
	RenderTile* mTile;
	StreamPipeline* mPipeline;
	const std::shared_ptr<LocalOutputQueue> mOutputQueue;
	const std::shared_ptr<LocalOutputSystem> mLocalSystem;
};
} // namespace PR