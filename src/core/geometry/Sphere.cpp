#include "Sphere.h"
#include "CollisionData.h"

#include "math/Spherical.h"

#include <utility>

namespace PR {
Sphere::Sphere()
	: mRadius(1)
{
}

Sphere::Sphere(float radius)
	: mRadius(radius)
{
	PR_ASSERT(radius > 0, "radius has to be bigger than 0. Check it before construction!");
}

void Sphere::intersects(const Ray& in, SingleCollisionOutput& out) const
{
	out.Successful = false;

	// C - O
	const float S  = -in.Origin.dot(in.Direction);
	const float L2 = in.Origin.squaredNorm();
	const float R2 = mRadius * mRadius;
	const float M2 = L2 - S * S;

	if ((S < 0 && // when object behind ray
		 L2 > R2)
		|| (M2 > R2))
		return;

	const float Q = std::sqrt(R2 - M2);

	float t0 = S - Q;
	float t1 = S + Q;
	if (t0 > t1)
		std::swap(t0, t1);

	if (!in.isInsideRange(t0))
		t0 = t1;

	if (in.isInsideRange(t0)) {
		out.HitDistance = t0;
		out.Successful	= true;

		// Setup UV
		Vector3f p		 = in.t(t0);
		Vector2f uv		 = project(p);
		out.Parameter[0] = uv(0);
		out.Parameter[1] = uv(1);
	}
}

void Sphere::intersects(const RayPackage& in, CollisionOutput& out) const
{
	const vfloat S	= -in.Origin.dot(in.Direction);
	const vfloat L2 = in.Origin.squaredNorm();
	const vfloat R2 = vfloat(mRadius * mRadius);
	const vfloat M2 = L2 - S * S;

	const bfloat valid = ((S >= 0) | (L2 <= R2)) & (M2 <= R2);

	const vfloat Q = sqrt(R2 - M2);

	const vfloat t0t = S - Q;
	const vfloat t1t = S + Q;
	const bfloat d	 = t0t > t1t;

	vfloat t0 = blend(t1t, t0t, d);
	vfloat t1 = blend(t0t, t1t, d);

	out.HitDistance = simdpp::blend(t1, t0, ~in.isInsideRange(t0));

	// Project
	Vector3fv p		 = in.t(t0);
	Vector2fv uv	 = project(p);
	out.Parameter[0] = uv(0);
	out.Parameter[1] = uv(1);

	out.Successful = valid & in.isInsideRange(out.HitDistance);
}

void Sphere::combine(const Vector3f& point)
{
	float f = point.squaredNorm();
	if (f > mRadius * mRadius)
		mRadius = std::sqrt(f);
}

void Sphere::combine(const Sphere& other)
{
	if (!isValid()) {
		*this = other;
		return;
	}

	mRadius = std::max(mRadius, other.mRadius);
}

Vector3f Sphere::normalPoint(float u, float v) const
{
	return Spherical::cartesian_from_uv<float>(u, v);
}

Vector3f Sphere::surfacePoint(float u, float v) const
{
	return normalPoint(u, v) * mRadius;
}

Vector2f Sphere::project(const Vector3f& p) const
{
	return Spherical::uv_from_point(p);
}

Vector2fv Sphere::project(const Vector3fv& p) const
{
	return Spherical::uv_from_point(p);
}
} // namespace PR
