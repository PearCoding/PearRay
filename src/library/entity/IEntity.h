#pragma once

#include "VirtualEntity.h"
#include "geometry/BoundingBox.h"

namespace PR {
class IMaterial;
class RenderContext;
struct FacePoint;
class Sampler;

class PR_LIB IEntity : public VirtualEntity {
public:
	IEntity(uint32 id, const std::string& name);
	virtual ~IEntity();

	bool isRenderable() const override;

	uint32 containerID() const;
	void setContainerID(uint32 id);

	inline BoundingBox worldBoundingBox() const;

	// Optional Interface
	virtual bool isCollidable() const;
	virtual float collisionCost() const;

	virtual std::string dumpInformation() const override;

	// Mandatory Interface
	virtual bool isLight() const				  = 0;
	virtual float surfaceArea(IMaterial* m) const = 0;

	virtual void getNormal(const vuint32& primID, const vfloat& u, const vfloat& v,
						   vfloat& n1, vfloat& n2, vfloat& n3) const = 0;

	virtual BoundingBox localBoundingBox() const = 0;

	virtual void checkCollision(const CollisionInput& in, CollisionOutput& out) const = 0;

protected:
	virtual void onFreeze(RenderContext* context) override;

private:
	uint32 mContainerID; // To which (latest) container node it belongs
	BoundingBox calcWorldBoundingBox() const;
	BoundingBox mWorldBoundingBox_Cache;
};
} // namespace PR

#include "IEntity.inl"
