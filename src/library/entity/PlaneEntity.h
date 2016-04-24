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

		void setPlane(const Plane& plane);
		Plane plane() const;

		bool isLight() const;
		void setMaterial(Material* m);
		Material* material() const;

		virtual bool isCollidable() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint) override;

		virtual void apply(Ray& in, const FacePoint& point, Renderer* renderer) override;

		virtual FacePoint getRandomFacePoint(Random& random) const;
	private:
		Plane mPlane;
		Material* mMaterial;
	};
}