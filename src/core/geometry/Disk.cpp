#include "Disk.h"
#include "CollisionData.h"

namespace PR {
Disk::Disk(float radius)
	: mRadius(radius)
{
}

bool Disk::contains(const Vector3f& point) const
{
	float r2 = point(0) * point(0) + point(1) * point(1);
	return r2 <= mRadius * mRadius;
}

void Disk::intersects(const Ray& in, SingleCollisionOutput& out) const
{
	out.Successful = false;
	if (abs(in.Direction(2)) <= PR_EPSILON) // Perpendicular
		return;

	const float t = -in.Origin(2) / in.Direction(2);
	if (!in.isInsideRange(t))
		return;

	const Vector3f p = in.t(t);

	float r2 = p(0) * p(0) + p(1) * p(1);
	if (r2 > mRadius * mRadius)
		return;

	out.Successful	= true;
	out.HitDistance = t;

	Vector2f proj	 = project(p);
	out.Parameter[0] = proj(0);
	out.Parameter[1] = proj(1);
}

void Disk::intersects(const RayPackage& in, CollisionOutput& out) const
{
	out.HitDistance	  = in.Origin(2) / in.Direction(2);
	const Vector3fv p = in.t(out.HitDistance);
	const vfloat r2	  = p(0) * p(0) + p(1) * p(1);

	Vector2fv proj	 = project(p);
	out.Parameter[0] = proj(0);
	out.Parameter[1] = proj(1);

	out.Successful = (r2 < mRadius * mRadius) & (abs(in.Direction(2)) > PR_EPSILON) & (in.isInsideRange(out.HitDistance));
}

Vector2f Disk::project(const Vector3f& point) const
{
	float r = sqrt(point(0) * point(0) + point(1) * point(1)) / mRadius;

	return Vector2f(
		0.5f * (atan2(point(1), point(0)) * PR_1_PI + 1),
		r);
}

Vector2fv Disk::project(const Vector3fv& point) const
{
	vfloat r = sqrt(point(0) * point(0) + point(1) * point(1)) / mRadius;

	return Vector2fv(
		0.5f * (atan2(point(1), point(0)) * PR_1_PI + 1),
		r);
}
} // namespace PR
