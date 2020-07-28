#include "Disk.h"
#include "trace/HitPoint.h"

namespace PR {
Disk::Disk(float radius)
	: mRadius(radius)
{
}

bool Disk::contains(const Vector3f& point) const
{
	float r2 = sumProd(point(0), point(0), point(1), point(1));
	return r2 <= mRadius * mRadius;
}

void Disk::intersects(const Ray& in, HitPoint& out) const
{
	out.resetSuccessful();
	if (abs(in.Direction(2)) <= PR_EPSILON) // Perpendicular
		return;

	const float t = -in.Origin(2) / in.Direction(2);
	if (!in.isInsideRange(t))
		return;

	const Vector3f p = in.t(t);

	float r2 = sumProd(p(0), p(0), p(1), p(1));
	if (r2 > mRadius * mRadius)
		return;

	out.makeSuccessful();
	out.HitDistance = t;

	Vector2f proj	 = project(p);
	out.Parameter[0] = proj(0);
	out.Parameter[1] = proj(1);
}

Vector2f Disk::project(const Vector3f& point) const
{
	float r = sqrt(sumProd(point(0), point(0), point(1), point(1))) / mRadius;

	return Vector2f(
		0.5f * (atan2(point(1), point(0)) * PR_INV_PI + 1),
		r);
}

} // namespace PR
