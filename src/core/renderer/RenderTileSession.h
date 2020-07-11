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

class RenderTile;
class IMaterial;
class IEmission;
class IntersectionPoint;
class LightPath;
class OutputQueue;
class FrameBufferBucket;

class PR_LIB_CORE RenderTileSession {
public:
	RenderTileSession(); // Dummy session!
	RenderTileSession(uint32 threadIndex, RenderTile* tile,
					  const std::shared_ptr<OutputQueue>& queue,
					  const std::shared_ptr<FrameBufferBucket>& bucket);
	~RenderTileSession();

	inline uint32 threadID() const
	{
		return mThread;
	}

	inline RenderTile* tile() const
	{
		return mTile;
	}

	IEntity* getEntity(uint32 id) const;
	IMaterial* getMaterial(uint32 id) const;
	IEmission* getEmission(uint32 id) const;

	bool traceBounceRay(const Ray& ray, Vector3f& pos, GeometryPoint& pt, IEntity*& entity, IMaterial*& material) const;
	bool traceShadowRay(const Ray& ray, float distance, uint32 entity_id) const;
	bool traceOcclusionRay(const Ray& ray) const;

	void pushSpectralFragment(const SpectralBlob& spec, const Ray& ray,
							  const LightPath& path) const;
	void pushSPFragment(const IntersectionPoint& pt, const LightPath& path) const;
	void pushFeedbackFragment(uint32 feedback, const Ray& ray) const;

	IEntity* sampleLight(const EntitySamplingInfo& info, uint32 ignore_id, const Vector3f& rnd, Vector3f& pos, GeometryPoint& pt, float& pdf) const;
	// FIXME: This is incomplete, as the resulting position is not taken into account
	float sampleLightPDF(const EntitySamplingInfo& info, uint32 ignore_id, IEntity* light) const;

private:
	Point2i localCoordinates(Point1i pixelIndex) const;

	uint32 currentIndex() const;

	uint32 mThread;
	RenderTile* mTile;
	std::shared_ptr<OutputQueue> mOutputQueue;
	std::shared_ptr<FrameBufferBucket> mBucket;
};
} // namespace PR