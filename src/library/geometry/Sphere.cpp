#include "Sphere.h"

#include "ray/Ray.h"

#include <utility>

#define PR_SPHERE_INTERSECT_EPSILON (1e-5f)
namespace PR
{
	Sphere::Sphere() :
		mPosition(PM::pm_Set(0,0,0,1)), mRadius(1)
	{
	}

	Sphere::Sphere(PM::vec3 pos, float radius) :
		mPosition(pos), mRadius(radius)
	{
		PR_ASSERT(radius > 0);
	}

	Sphere::Sphere(const Sphere& other)
	{
		mPosition = other.mPosition;
		mRadius = other.mRadius;
	}

	Sphere& Sphere::operator = (const Sphere& other)
	{
		mPosition = other.mPosition;
		mRadius = other.mRadius;
		return *this;
	}

	bool Sphere::intersects(const Ray& ray, PM::vec3& collisionPoint, float& t) const
	{
		const PM::vec3 L = PM::pm_Subtract(mPosition, ray.startPosition()); // C - O
		const float S = PM::pm_Dot3D(L, ray.direction()); // L . D
		const float L2 = PM::pm_MagnitudeSqr3D(L); // L . L
		const float R2 = mRadius*mRadius; // R^2

		if (S < 0 && // when object behind ray
			L2 > R2)
			return false;

		const float M2 = L2 - S*S; // L . L - S^2

		if (M2 > R2)
			return false;

		const float Q = std::sqrt(R2 - M2);
		
		float t0 = S - Q;
		float t1 = S + Q;
		if (t0 > t1)
			std::swap(t0, t1);

		if (t0 < PR_SPHERE_INTERSECT_EPSILON)
			t0 = t1;

		if (t0 >= PR_SPHERE_INTERSECT_EPSILON)
		{
			t = t0;
			collisionPoint = PM::pm_Add(ray.startPosition(), PM::pm_Scale(ray.direction(), t));
			return true;
		}
		else
		{
			return false;
		}
	}

	void Sphere::put(const PM::vec3& point)
	{
		float f = PM::pm_Magnitude3D(PM::pm_Subtract(mPosition, point));
		if (f > mRadius)
			mRadius = f;
	}

	void Sphere::combine(const Sphere& other)
	{
		if (!isValid())
		{
			*this = other;
			return;
		}

		float f = PM::pm_Magnitude3D(PM::pm_Subtract(mPosition, other.mPosition)) + other.mRadius;
		if (f > mRadius)
			mRadius = f;
	}
}