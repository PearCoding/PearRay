#pragma once

#include "RenderEntity.h"

namespace PR {
class Material;
class PR_LIB BoundaryEntity : public RenderEntity {
public:
	ENTITY_CLASS

	BoundaryEntity(uint32 id, const std::string& name, const BoundingBox& box);
	virtual ~BoundaryEntity();

	std::string type() const override;

	bool isLight() const override;
	float surfaceArea(Material* m) const override;

	void setMaterial(const std::shared_ptr<Material>& m);
	std::shared_ptr<Material> material() const;

	void setBoundingBox(const BoundingBox& box);

	bool isCollidable() const override;
	float collisionCost() const override;
	BoundingBox localBoundingBox() const override;

	RenderEntity::Collision checkCollision(const Ray& ray) const override;
	RenderEntity::FacePointSample sampleFacePoint(const Eigen::Vector3f& rnd) const override;

protected:
	// Entity
	void onFreeze(RenderContext* context) override;

private:
	BoundingBox mBoundingBox;
	std::shared_ptr<Material> mMaterial;

	float mPDF_Cache[6];
};
}
