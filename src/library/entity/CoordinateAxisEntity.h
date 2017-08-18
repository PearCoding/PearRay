#pragma once

#include "RenderEntity.h"

namespace PR {
class Material;
class PR_LIB CoordinateAxisEntity : public RenderEntity {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	CoordinateAxisEntity(uint32 id, const std::string& name);
	virtual ~CoordinateAxisEntity();

	std::string type() const override;

	bool isLight() const override;
	float surfaceArea(Material* m) const override;

	void setAxisLength(float f);
	float axisLength() const;

	void setAxisThickness(float f);
	float axisThickness() const;

	void setXMaterial(const std::shared_ptr<Material>& m);
	const std::shared_ptr<Material>& xMaterial() const;

	void setYMaterial(const std::shared_ptr<Material>& m);
	const std::shared_ptr<Material>& yMaterial() const;

	void setZMaterial(const std::shared_ptr<Material>& m);
	const std::shared_ptr<Material>& zMaterial() const;

	bool isCollidable() const override;
	float collisionCost() const override;
	BoundingBox localBoundingBox() const override;

	RenderEntity::Collision checkCollision(const Ray& ray) const override;

	RenderEntity::FacePointSample sampleFacePoint(const Eigen::Vector3f& rnd, uint32 sample) const override;

	// Entity
	void onFreeze() override;

	// RenderEntity
	void setup(RenderContext* context) override;

private:
	void setup_cache() const;

	float mAxisLength;
	float mAxisThickness;

	std::shared_ptr<Material> mMaterials[3];

	mutable BoundingBox mBoundingBox_Cache;
	mutable BoundingBox mAxisBoundingBox_Cache[3];
	mutable float mPDF_Cache;
};
}
