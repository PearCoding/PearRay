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
	RenderTileSession(); // Dummy session!
	RenderTileSession(uint32 threadIndex, RenderTile* tile,
					  const std::shared_ptr<OutputBufferBucket>& bucket);
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

	bool traceBounceRay(const Ray& ray, GeometryPoint& pt, IEntity*& entity, IMaterial*& material) const;
	ShadowHit traceShadowRay(const Ray& ray) const;
	bool traceOcclusionRay(const Ray& ray) const;

	void pushSpectralFragment(const SpectralBlob& spec, const Ray& ray,
							  const LightPath& path) const;
	void pushSPFragment(const ShadingPoint& pt, const LightPath& path) const;
	void pushFeedbackFragment(uint32 feedback, const Ray& ray) const;

	IEntity* pickRandomLight(const Vector3f& view, GeometryPoint& pt, float& pdf) const;
private:
	Point2i localCoordinates(Point1i pixelIndex) const;

	uint32 currentIndex() const;

	uint32 mThread;
	RenderTile* mTile;
	std::shared_ptr<OutputBufferBucket> mBucket;
};
} // namespace PR