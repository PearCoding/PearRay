#pragma once

#include "Entity.h"
#include "geometry/BoundingBox.h"
#include "shader/FacePoint.h"

namespace PR {
class Material;
class Ray;
class RenderContext;
struct FacePoint;
class Sampler;

class PR_LIB RenderEntity : public Entity {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	RenderEntity(uint32 id, const std::string& name);
	virtual ~RenderEntity();

	bool isRenderable() const override;

	virtual bool isLight() const				 = 0;
	virtual float surfaceArea(Material* m) const = 0;

	virtual bool isCollidable() const;
	virtual float collisionCost() const;

	virtual BoundingBox localBoundingBox() const = 0;
	inline BoundingBox worldBoundingBox() const;

	struct Collision {
		bool Successful;
		FacePoint Point;
	};
	// In world coords
	virtual Collision checkCollision(const Ray& ray) const = 0;

	struct FacePointSample {
		float PDF;
		FacePoint Point;
	};
	// In world coords
	virtual FacePointSample sampleFacePoint(const Eigen::Vector3f& rnd, uint32 sample) const = 0;

	// Entity
	void onFreeze() override;
	std::string dumpInformation() const override;

	virtual void setup(RenderContext* context) {}

private:
	BoundingBox calcWorldBoundingBox() const;
	BoundingBox mWorldBoundingBox_Cache;
};
}

#include "RenderEntity.inl"
