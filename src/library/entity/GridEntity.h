#pragma once

#include "RenderEntity.h"
#include "geometry/Plane.h"

namespace PR
{
	class Material;
	class PR_LIB GridEntity : public RenderEntity
	{
	public:
		GridEntity(const std::string& name, const Plane& plane, Entity* parent = nullptr);
		virtual ~GridEntity();

		virtual std::string type() const;

		void setPlane(const Plane& plane);
		Plane plane() const;

		void setGridCount(int f);
		int gridCount() const;

		bool isLight() const;
		void setFirstMaterial(Material* m);
		Material* firstMaterial() const;
		void setSecondMaterial(Material* m);
		Material* secondMaterial() const;

		virtual bool isCollidable() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint) override;

		virtual void apply(Ray& in, const FacePoint& point, Renderer* renderer) override;

		virtual FacePoint getRandomFacePoint(Random& random) const;
	private:
		Plane mPlane;
		
		Material* mFirstMaterial;
		Material* mSecondMaterial;

		int mGridCount;
	};
}