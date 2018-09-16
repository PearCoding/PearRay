#pragma once

#include "VirtualEntity.h"
#include "geometry/BoundingBox.h"
#include "shader/FacePoint.h"

namespace PR {
class Material;
class Ray;
class RenderContext;
struct FacePoint;
class Sampler;

class PR_LIB RenderEntity : public VirtualEntity {
public:
	ENTITY_CLASS

	RenderEntity(uint32 id, const std::string& name);
	virtual ~RenderEntity();

	bool isRenderable() const override;

	virtual bool isLight() const				 = 0;
	virtual float surfaceArea(Material* m) const = 0;

	virtual bool isCollidable() const;
	virtual float collisionCost() const;

	uint32 containerID() const;
	void setContainerID(uint32 id);

	virtual void getNormal(vuint32 primID, vfloat u, vfloat v,
						   vfloat& n1, vfloat& n2, vfloat& n3) const {};

	virtual BoundingBox localBoundingBox() const = 0;
	inline BoundingBox worldBoundingBox() const;

	struct Collision {
		bool Successful;
		FacePoint Point;
	};
	// In world coords
	virtual Collision checkCollision(const Ray& ray) const = 0;

	virtual void checkCollisionV(const CollisionInput& in, CollisionOutput& out) const = 0;

	struct FacePointSample {
		float PDF_A; // Respect to Area
		FacePoint Point;
	};
	// In world coords
	virtual FacePointSample sampleFacePoint(const Eigen::Vector3f& rnd) const = 0;

	// VirtualEntity
	std::string dumpInformation() const override;

protected:
	virtual void onFreeze(RenderContext* context) override;

private:
	uint32 mContainerID; // To which (latest) container node it belongs
	BoundingBox calcWorldBoundingBox() const;
	BoundingBox mWorldBoundingBox_Cache;
};
} // namespace PR

#include "RenderEntity.inl"
