#pragma once

#include "VirtualEntity.h"
#include "geometry/BoundingBox.h"

namespace PR {
class GeometryPoint;
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
	virtual bool isLight() const						   = 0;
	virtual float surfaceArea(uint32 materialID = 0) const = 0;

	virtual BoundingBox localBoundingBox() const = 0;

	virtual void checkCollision(const Ray& in, SingleCollisionOutput& out) const  = 0;
	virtual void checkCollision(const RayPackage& in, CollisionOutput& out) const = 0;

	virtual Vector2f pickRandomPoint(const Vector2f& rnd,
									 uint32& faceID, float& pdf) const = 0;
	virtual void provideGeometryPoint(uint32 faceID, float u, float v,
									  GeometryPoint& pt) const		   = 0;

protected:
	virtual void onFreeze(RenderContext* context) override;

private:
	uint32 mContainerID; // To which (latest) container node it belongs
	BoundingBox calcWorldBoundingBox() const;
	BoundingBox mWorldBoundingBox_Cache;
};
} // namespace PR

#include "IEntity.inl"
