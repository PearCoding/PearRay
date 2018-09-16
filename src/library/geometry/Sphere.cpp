#include "Sphere.h"
#include "CollisionData.h"

#include "ray/Ray.h"
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

Sphere::Intersection Sphere::intersects(const Ray& ray) const
{
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
		r.Position   = ray.origin() + ray.direction() * r.T;
		r.Successful = true;
		return r;
	} else {
		return r;
	}
}

void Sphere::intersectsV(const CollisionInput& in, CollisionOutput& out) const
{
	using namespace simdpp;

	// C - O
	const vfloat Lx = mPosition(0) - in.RayOrigin[0];
	const vfloat Ly = mPosition(1) - in.RayOrigin[1];
	const vfloat Lz = mPosition(2) - in.RayOrigin[2];

	const vfloat S  = dotV(Lx, Ly, Lz, in.RayDirection[0], in.RayDirection[1], in.RayDirection[2]); // L . D
	const vfloat L2 = dotV(Lx, Ly, Lz, Lx, Ly, Lz);												 // L . L
	const float R2	= mRadius * mRadius;															 // R^2
	const vfloat M2 = L2 - S * S;																	 // L . L - S^2

	const bfloat valid = (S >= 0) | (L2 <= R2) | (M2 <= R2);

	const vfloat Q = sqrt(R2 - M2);

	const vfloat t0t	= S - Q;
	const vfloat t1t	= S + Q;
	const bfloat d = t0t > t1t;

	vfloat t0 = blend(t1t, t0t, d);
	vfloat t1 = blend(t0t, t1t, d);

	out.HitDistance = blend(t1, t0, t0 < PR_SPHERE_INTERSECT_EPSILON);

	const vfloat inf = make_float(std::numeric_limits<float>::infinity());
	out.HitDistance	= blend(out.HitDistance, inf, valid);
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
} // namespace PR
