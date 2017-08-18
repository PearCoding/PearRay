#pragma once

#include "RenderEntity.h"

namespace PR {
class Material;
class PR_LIB SphereEntity : public RenderEntity {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	SphereEntity(uint32 id, const std::string& name, float r);
	virtual ~SphereEntity();

	std::string type() const;

	bool isLight() const override;
	float surfaceArea(Material* m) const override;

	void setMaterial(const std::shared_ptr<Material>& m);
	const std::shared_ptr<Material>& material() const;

	void setRadius(float f);
	float radius() const;

	bool isCollidable() const override;
	float collisionCost() const override;
	BoundingBox localBoundingBox() const override;

	RenderEntity::Collision checkCollision(const Ray& ray) const override;
	RenderEntity::FacePointSample sampleFacePoint(const Eigen::Vector3f& rnd, uint32 sample) const override;

	// RenderEntity
	void setup(RenderContext* context) override;

private:
	float mRadius;
	std::shared_ptr<Material> mMaterial;

	float mPDF_Cache;
};
}
