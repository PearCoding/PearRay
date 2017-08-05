#pragma once

#include "RenderEntity.h"

namespace PR {
class Material;
class PR_LIB BoundaryEntity : public RenderEntity {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	BoundaryEntity(uint32 id, const std::string& name, const BoundingBox& box);
	virtual ~BoundaryEntity();

	std::string type() const override;

	bool isLight() const override;
	float surfaceArea(Material* m) const override;

	void setMaterial(const std::shared_ptr<Material>& m);
	const std::shared_ptr<Material>& material() const;

	void setBoundingBox(const BoundingBox& box);

	bool isCollidable() const override;
	float collisionCost() const override;
	BoundingBox localBoundingBox() const override;

	RenderEntity::Collision checkCollision(const Ray& ray) const override;
	RenderEntity::FacePointSample sampleFacePoint(Sampler& sampler, uint32 sample) const override;

	// RenderEntity
	void setup(RenderContext* context) override;

private:
	BoundingBox mBoundingBox;
	std::shared_ptr<Material> mMaterial;
};
}
