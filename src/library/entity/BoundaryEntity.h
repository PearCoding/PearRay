#pragma once

#include "RenderEntity.h"

namespace PR
{
	class Material;
	class PR_LIB BoundaryEntity : public RenderEntity
	{
	public:
		BoundaryEntity(const std::string& name, const BoundingBox& box, Entity* parent = nullptr);
		virtual ~BoundaryEntity();

		virtual std::string type() const override;

		virtual bool isLight() const override;

		void setMaterial(Material* m);
		Material* material() const;

		void setBoundingBox(const BoundingBox& box);

		virtual bool isCollidable() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t) override;

		virtual FacePoint getRandomFacePoint(Sampler& sampler, uint32 sample) const;
	private:
		BoundingBox mBoundingBox;
		Material* mMaterial;
	};
}