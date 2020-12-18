#pragma once

#include "Random.h"
#include "buffer/Feedback.h"
#include "entity/IEntity.h"
#include "geometry/GeometryPoint.h"
#include "ray/Ray.h"
#include "ray/RayStream.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileStatistics.h"
#include "scene/Scene.h"
#include "trace/HitStream.h"

namespace PR {

class FrameBufferBucket;
class IEmission;
class IMaterial;
class IntersectionPoint;
class LightPath;
class OutputQueue;
class RayGroup;
class RenderTile;
class StreamPipeline;

class PR_LIB_CORE RenderTileSession {
public:
	RenderTileSession(); // Dummy session!
	RenderTileSession(uint32 threadIndex, RenderTile* tile, StreamPipeline* pipeline,
					  const std::shared_ptr<OutputQueue>& queue,
					  const std::shared_ptr<FrameBufferBucket>& bucket);
	~RenderTileSession();

	inline uint32 threadID() const { return mThread; }

	inline RenderTile* tile() const { return mTile; }
	inline const RenderContext* context() const { return mTile->context(); }
	inline Random& random() { return mTile->random(); }
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

private:
	Point2i localCoordinates(Point1i pixelIndex) const;

	uint32 currentIndex() const;

	uint32 mThread;
	RenderTile* mTile;
	StreamPipeline* mPipeline;
	std::shared_ptr<OutputQueue> mOutputQueue;
	std::shared_ptr<FrameBufferBucket> mBucket;
};
} // namespace PR