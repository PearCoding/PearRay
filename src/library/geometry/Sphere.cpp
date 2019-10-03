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

void Sphere::intersects(const Ray& in, SingleCollisionOutput& out) const
{
	out.HitDistance = std::numeric_limits<float>::infinity();

	// C - O
	const float Lx = mPosition(0) - in.Origin[0];
	const float Ly = mPosition(1) - in.Origin[1];
	const float Lz = mPosition(2) - in.Origin[2];

	const float S  = dotV(Lx, Ly, Lz, in.Direction[0], in.Direction[1], in.Direction[2]); // L . D
	const float L2 = dotV(Lx, Ly, Lz, Lx, Ly, Lz);										  // L . L
	const float R2 = mRadius * mRadius;													  // R^2
	const float M2 = L2 - S * S;														  // L . L - S^2

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
	}
}

void Sphere::intersects(const RayPackage& in, CollisionOutput& out) const
{
	using namespace simdpp;

	// C - O
	const vfloat Lx = mPosition(0) - in.Origin[0];
	const vfloat Ly = mPosition(1) - in.Origin[1];
	const vfloat Lz = mPosition(2) - in.Origin[2];

	const vfloat S  = dotV(Lx, Ly, Lz, in.Direction[0], in.Direction[1], in.Direction[2]); // L . D
	const vfloat L2 = dotV(Lx, Ly, Lz, Lx, Ly, Lz);										   // L . L
	const float R2  = mRadius * mRadius;												   // R^2
	const vfloat M2 = L2 - S * S;														   // L . L - S^2

	const bfloat valid = ((S >= 0) | (L2 <= R2)) & (M2 <= R2);

	const vfloat Q = sqrt(R2 - M2);

	const vfloat t0t = S - Q;
	const vfloat t1t = S + Q;
	const bfloat d   = t0t > t1t;

	vfloat t0 = blend(t1t, t0t, d);
	vfloat t1 = blend(t0t, t1t, d);

	out.HitDistance = blend(t1, t0, t0 < PR_SPHERE_INTERSECT_EPSILON);

	const vfloat inf = make_float(std::numeric_limits<float>::infinity());
	out.HitDistance  = blend(out.HitDistance, inf, valid);
}

void Sphere::combine(const Eigen::Vector3f& point)
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

Eigen::Vector3f Sphere::normalPoint(float u, float v) const
{
	return Projection::sphere_coord(u * 2 * M_PI, v * M_PI);
}

Eigen::Vector3f Sphere::surfacePoint(float u, float v) const
{
	return normalPoint(u, v) * mRadius;
}
} // namespace PR
