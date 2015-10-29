#include "SphereEntity.h"
#include "ray/Ray.h"

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

	bool SphereEntity::checkCollision(const Ray& ray, PM::vec3& collisionPoint)
	{
		const PM::vec3 sc = PM::pm_Subtract(ray.startPosition(), position()); // S - C
		// const float A = PM::pm_MagnitudeSqr3D(ray.direction());		// D^2 -> ASSUSE 1, for normalization
		const float B = 2 * PM::pm_Dot3D(ray.direction(), sc);		// D . (S - C)
		const float C = PM::pm_MagnitudeSqr3D(sc) - mRadius*mRadius;// (S - C)^2 - R^2
		
		const float d = B*B - 4 * C;

		if (d < 0)
		{
			return false;
		}

		const float s = sqrtf(B*B - 4 * C);
		const float t0 = abs((-B - s)) / 2;
		const float t1 = abs((-B + s)) / 2;

		collisionPoint = PM::pm_Add(ray.startPosition(), PM::pm_Scale(ray.direction(), t0 < t1 ? t0 : t1));
		return true;
	}
}