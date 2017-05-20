#pragma once

#include "RenderEntity.h"
#include "geometry/Plane.h"

namespace PR
{
	class Material;
	class PR_LIB PlaneEntity : public RenderEntity
	{
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		
		PlaneEntity(uint32 id, const std::string& name, const Plane& plane);
		virtual ~PlaneEntity();

		virtual std::string type() const;

		virtual bool isLight() const override;
		virtual float surfaceArea(Material* m) const override;

		void setMaterial(const std::shared_ptr<Material>& m);
		const std::shared_ptr<Material>& material() const;

		void setPlane(const Plane& plane);
		const Plane& plane() const;

		virtual bool isCollidable() const override;
		virtual float collisionCost() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FaceSample& collisionPoint) const override;

		virtual FaceSample getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const override;

		// Entity
		virtual void onFreeze() override;

		// RenderEntity
		virtual void setup(RenderContext* context) override;
	private:
		Plane mPlane;
		std::shared_ptr<Material> mMaterial;

		Plane mGlobalPlane_Cache;
	};
}
