#include "Sphere.h"
#include "CollisionData.h"

#include "math/Projection.h"
#include "math/SIMD.h"

#include <utility>

#define PR_SPHERE_INTERSECT_EPSILON (1e-5f)
namespace PR {
Sphere::Sphere()
	: mPosition(0, 0, 0)
	, mRadius(1)
{
}

Sphere::Sphere(const Vector3f& pos, float radius)
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

void Sphere::intersects(const Ray& in, SingleCollisionOutput& out) const
{
	out.HitDistance = std::numeric_limits<float>::infinity();

	// C - O
	const Vector3f L = mPosition - in.Origin;

	const float S  = L.dot(in.Direction);
	const float L2 = L.squaredNorm();
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

	if (t0 < PR_SPHERE_INTERSECT_EPSILON)
		t0 = t1;

	if (t0 >= PR_SPHERE_INTERSECT_EPSILON) {
		out.HitDistance = t0;

		// Setup UV
		Vector3f p  = in.t(t0);
		Vector2f uv = project(p);
		out.UV[0]   = uv(0);
		out.UV[1]   = uv(1);
	}
}

void Sphere::intersects(const RayPackage& in, CollisionOutput& out) const
{
	using namespace simdpp;

	const Vector3fv L = promote(mPosition) - in.Origin;
	const vfloat S	= L.dot(in.Direction);
	const vfloat L2   = L.squaredNorm();
	const float R2	= mRadius * mRadius;
	const vfloat M2   = L2 - S * S;

	const bfloat valid = ((S >= 0) | (L2 <= R2)) & (M2 <= R2);

	const vfloat Q = sqrt(R2 - M2);

	const vfloat t0t = S - Q;
	const vfloat t1t = S + Q;
	const bfloat d   = t0t > t1t;

	vfloat t0 = blend(t1t, t0t, d);
	vfloat t1 = blend(t0t, t1t, d);

	out.HitDistance = blend(t1, t0, t0 < PR_SPHERE_INTERSECT_EPSILON);

	// Project
	Vector3fv p  = in.t(t0);
	Vector2fv uv = project(p);
	out.UV[0]	= uv(0);
	out.UV[1]	= uv(1);

	const vfloat inf = fill_vector(std::numeric_limits<float>::infinity());
	out.HitDistance  = blend(out.HitDistance, inf,
							 valid & (out.HitDistance >= PR_SPHERE_INTERSECT_EPSILON));
}

void Sphere::combine(const Vector3f& point)
{
	float f = (mPosition - point).squaredNorm();
	if (f > mRadius * mRadius)
		mRadius = std::sqrt(f);
}

void Sphere::combine(const Sphere& other)
{
	if (!isValid()) {
		*this = other;
		return;
	}

	float f = (mPosition - other.mPosition).squaredNorm() + other.mRadius;
	if (f > mRadius * mRadius)
		mRadius = std::sqrt(f);
}

Vector3f Sphere::normalPoint(float u, float v) const
{
	return Projection::sphere_coord(u * 2 * M_PI, v * M_PI);
}

Vector3f Sphere::surfacePoint(float u, float v) const
{
	return normalPoint(u, v) * mRadius;
}

Vector2f Sphere::project(const Vector3f& p) const
{
	return Projection::sphereUV<float>(p - mPosition);
}

Vector2fv Sphere::project(const Vector3fv& p) const
{
	return Projection::sphereUV<vfloat>(p - promote(mPosition));
}
} // namespace PR
