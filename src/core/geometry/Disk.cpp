#include "Disk.h"
#include "Triangle.h"
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

void Disk::triangulate(const Vector3f& center, float radius, uint32 sectionCount, std::vector<float>& vertices)
{
	sectionCount = std::max<uint32>(3, sectionCount);

	Vector3f Nx = Vector3f(1, 0, 0);
	Vector3f Ny = Vector3f(0, 1, 0);

	auto add = [&](const Vector3f& v) {vertices.push_back(v[0]); vertices.push_back(v[1]); vertices.push_back(v[2]); };

	add(center);

	float step = 1.0f / sectionCount;
	for (uint32 i = 0; i < sectionCount; ++i) {
		float x = std::cos(2 * 3.141592f * step * i);
		float y = std::sin(2 * 3.141592f * step * i);

		add(radius * Nx * x + radius * Ny * y + center);
	}
}

void Disk::triangulateIndices(uint32 centerID, uint32 sectionCount, std::vector<uint32>& indices, uint32 off)
{
	sectionCount = std::max<uint32>(3, sectionCount);

	for (uint32 i = 0; i < sectionCount; ++i) {
		uint32 C  = i + off;
		uint32 NC = (i + 1 < sectionCount ? i + 1 : 0) + off;

		Triangle::triangulateIndices({ centerID, C, NC }, indices);
	}
}
} // namespace PR
