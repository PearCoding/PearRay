#pragma once

#include "RenderEntity.h"
#include "geometry/Plane.h"

namespace PR {
class Material;
class PR_LIB PlaneEntity : public RenderEntity {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	PlaneEntity(uint32 id, const std::string& name, const Plane& plane);
	virtual ~PlaneEntity();

	std::string type() const;

	bool isLight() const override;
	float surfaceArea(Material* m) const override;

	void setMaterial(const std::shared_ptr<Material>& m);
	const std::shared_ptr<Material>& material() const;

	void setPlane(const Plane& plane);
	const Plane& plane() const;

	bool isCollidable() const override;
	float collisionCost() const override;
	BoundingBox localBoundingBox() const override;
	bool checkCollision(const Ray& ray, FaceSample& collisionPoint) const override;

	FaceSample getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const override;

	// Entity
	void onFreeze() override;

	// RenderEntity
	void setup(RenderContext* context) override;

private:
	Plane mPlane;
	std::shared_ptr<Material> mMaterial;

	Plane mGlobalPlane_Cache;
	Eigen::Vector3f mXAxisN_Cache;
	Eigen::Vector3f mYAxisN_Cache;
};
}
