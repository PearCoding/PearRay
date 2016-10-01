#pragma once

#include "RenderEntity.h"

namespace PR
{
	class Material;
	class PR_LIB BoundaryEntity : public RenderEntity
	{
	public:
		BoundaryEntity(const std::string& name, const BoundingBox& box);
		virtual ~BoundaryEntity();

		virtual std::string type() const override;

		virtual bool isLight() const override;
		virtual float surfaceArea(Material* m) const override;

		void setMaterial(Material* m);
		Material* material() const;

		void setBoundingBox(const BoundingBox& box);

		virtual bool isCollidable() const override;
		virtual float collisionCost() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FaceSample& collisionPoint) const override;

		virtual FaceSample getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const override;
	private:
		BoundingBox mBoundingBox;
		Material* mMaterial;
	};
}