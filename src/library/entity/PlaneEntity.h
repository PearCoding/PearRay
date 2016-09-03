#pragma once

#include "RenderEntity.h"
#include "geometry/Plane.h"

namespace PR
{
	class Material;
	class PR_LIB PlaneEntity : public RenderEntity
	{
	public:
		PlaneEntity(const std::string& name, const Plane& plane, Entity* parent = nullptr);
		virtual ~PlaneEntity();

		virtual std::string type() const;

		virtual bool isLight() const override;
		virtual float surfaceArea(Material* m) const override;

		void setMaterial(Material* m);
		Material* material() const;

		void setPlane(const Plane& plane);
		Plane plane() const;

		virtual bool isCollidable() const override;
		virtual float collisionCost() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, SamplePoint& collisionPoint) override;

		virtual SamplePoint getRandomFacePoint(Sampler& sampler, uint32 sample) const;
	private:
		Plane mPlane;
		Material* mMaterial;
	};
}