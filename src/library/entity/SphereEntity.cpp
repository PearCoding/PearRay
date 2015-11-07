#include "SphereEntity.h"
#include "ray/Ray.h"
#include "material/Material.h"
#include "geometry/FacePoint.h"

namespace PR
{
	SphereEntity::SphereEntity(const std::string& name, float r, Entity* parent) :
		GeometryEntity(name, parent), mRadius(r)
	{
	}

	SphereEntity::~SphereEntity()
	{
	}

	void SphereEntity::setRadius(float f)
	{
		mRadius = f;
	}

	float SphereEntity::radius() const
	{
		return mRadius;
	}

	void SphereEntity::setMaterial(Material* m)
	{
		mMaterial = m;
	}

	Material* SphereEntity::material() const
	{
		return mMaterial;
	}

	bool SphereEntity::isCollidable() const
	{
		return true;
	}

	BoundingBox SphereEntity::localBoundingBox() const
	{
		return BoundingBox(PM::pm_Set(mRadius, mRadius, mRadius, 1),
			PM::pm_Set(-mRadius, -mRadius, -mRadius, 1));
	}

	bool SphereEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint)
	{
		const PM::vec3 L = PM::pm_Subtract(position(), ray.startPosition()); // C - O
		const float S = PM::pm_Dot3D(L, ray.direction()); // L . D
		const float L2 = PM::pm_MagnitudeSqr3D(L); // L . L
		const float R2 = mRadius*mRadius; // R^2

		if (S < 0 && // when object behind ray
			L2 > R2) 
		{
			return false;
		}

		const float M2 = L2 - S*S; // L . L - S^2

		if (M2 > R2)
		{
			return false;
		}
		
		const float Q = sqrtf(R2 - M2);
		
		collisionPoint.setVertex(PM::pm_Add(ray.startPosition(), PM::pm_Scale(ray.direction(), L2 > R2 ? S - Q : S + Q)));

		PM::vec3 norm = PM::pm_Normalize3D(PM::pm_Subtract(collisionPoint.vertex(), position()));
		collisionPoint.setNormal(norm);

		return true;
	}

	void SphereEntity::apply(Ray& in, const FacePoint& point, Renderer* renderer)
	{
		if (mMaterial)
		{
			mMaterial->apply(in, this, point, renderer);
		}
	}
}