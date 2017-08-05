#include "Sphere.h"

#include "ray/Ray.h"

#include <utility>

#include "performance/Performance.h"

#define PR_SPHERE_INTERSECT_EPSILON (1e-5f)
namespace PR {
Sphere::Sphere()
	: mPosition(0, 0, 0)
	, mRadius(1)
{
}

Sphere::Sphere(const Eigen::Vector3f& pos, float radius)
	: mPosition(pos)
	, mRadius(radius)
{
	PR_ASSERT(radius > 0, "radius has to be bigger than 0. Check it before construction!");
}

Sphere::Sphere(const Sphere& other)
{
	mPosition = other.mPosition;
	mRadius   = other.mRadius;
}

Sphere& Sphere::operator=(const Sphere& other)
{
	mPosition = other.mPosition;
	mRadius   = other.mRadius;
	return *this;
}

Sphere::Intersection Sphere::intersects(const Ray& ray) const
{
	PR_GUARD_PROFILE();
	Sphere::Intersection r;
	r.Successful = false;

	const Eigen::Vector3f L = mPosition - ray.origin(); // C - O
	const float S			= L.dot(ray.direction());   // L . D
	const float L2			= L.squaredNorm();			// L . L
	const float R2			= mRadius * mRadius;		// R^2

	if (S < 0 && // when object behind ray
		L2 > R2)
		return r;

	const float M2 = L2 - S * S; // L . L - S^2

	if (M2 > R2)
		return r;

	const float Q = std::sqrt(R2 - M2);

	float t0 = S - Q;
	float t1 = S + Q;
	if (t0 > t1)
		std::swap(t0, t1);

	if (t0 < PR_SPHERE_INTERSECT_EPSILON)
		t0 = t1;

	if (t0 >= PR_SPHERE_INTERSECT_EPSILON) {
		r.T			 = t0;
		r.Position		 = ray.origin() + ray.direction() * r.T;
		r.Successful = true;
		return r;
	} else {
		return r;
	}
}

void Sphere::combine(const Eigen::Vector3f& point)
{
	PR_GUARD_PROFILE();

	float f = (mPosition - point).squaredNorm();
	if (f > mRadius * mRadius)
		mRadius = std::sqrt(f);
}

void Sphere::combine(const Sphere& other)
{
	PR_GUARD_PROFILE();

	if (!isValid()) {
		*this = other;
		return;
	}

	float f = (mPosition - other.mPosition).squaredNorm() + other.mRadius;
	if (f > mRadius * mRadius)
		mRadius = std::sqrt(f);
}
}
