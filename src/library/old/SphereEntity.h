#pragma once

#include "RenderEntity.h"

namespace PR {
class Material;
class PR_LIB SphereEntity : public RenderEntity {
public:
	ENTITY_CLASS

	SphereEntity(uint32 id, const std::string& name, float r);
	virtual ~SphereEntity();

	std::string type() const;

	bool isLight() const override;
	float surfaceArea(Material* m) const override;

	void setMaterial(const std::shared_ptr<Material>& m);
	std::shared_ptr<Material> material() const;

	void setRadius(float f);
	float radius() const;

	bool isCollidable() const override;
	float collisionCost() const override;
	BoundingBox localBoundingBox() const override;

	RenderEntity::Collision checkCollision(const Ray& ray) const override;
	RenderEntity::FacePointSample sampleFacePoint(const Eigen::Vector3f& rnd) const override;	void checkCollisionV(const CollisionInput& in, CollisionOutput& out) const override;

protected:
	void onFreeze(RenderContext* context) override;

private:
	float mRadius;
	std::shared_ptr<Material> mMaterial;

	float mPDF_Cache;
};
}
