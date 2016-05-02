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

		virtual std::string type() const;

		void setBoundingBox(const BoundingBox& box);

		virtual bool isCollidable() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint) override;

		virtual FacePoint getRandomFacePoint(Sampler& sampler, Random& random) const;
	private:
		BoundingBox mBoundingBox;
	};
}