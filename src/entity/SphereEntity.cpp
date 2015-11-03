#include "SphereEntity.h"
#include "ray/Ray.h"
#include "material/Material.h"
#include "geometry/FacePoint.h"

namespace PR
{
	SphereEntity::SphereEntity(const std::string& name, float r, Entity* parent) :
		Entity(name, parent), mRadius(r)
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

	BoundingBox SphereEntity::boundingBox() const
	{
		return BoundingBox(PM::pm_Set(mRadius, mRadius, mRadius), PM::pm_Set(-mRadius, -mRadius, -mRadius));
	}

	// TODO: Should handle ray.startpoint as well!
	bool SphereEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint)
	{
		const PM::vec3 sc = PM::pm_Subtract(ray.startPosition(), position()); // S - C

		// const float A = PM::pm_MagnitudeSqr3D(ray.direction());		// D^2 -> ASSUME 1, for normalization
		const float B = 2 * PM::pm_Dot3D(ray.direction(), sc);		// D . (S - C)
		const float C = PM::pm_MagnitudeSqr3D(sc) - mRadius*mRadius;// (S - C)^2 - R^2
		
		const float d = B*B - 4 * C;

		if (d < 0)
		{
			return false;
		}

		const float s = sqrtf(B*B - 4 * C);
		const float t0 = (-B - s) / 2;
		const float t1 = (-B + s) / 2;

		if (t0 < 0 && t1 < 0)
		{
			return false;
		}
		
		float t = t0;
		if (t0 < 0 && t1 >= 0)
		{
			t = t1;
		}
		else if (t0 > 0 && t1 >= 0)
		{
			t = t0 < t1 ? t0 : t1;
		}

		collisionPoint.setVertex(PM::pm_Add(ray.startPosition(), PM::pm_Scale(ray.direction(), t)));

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