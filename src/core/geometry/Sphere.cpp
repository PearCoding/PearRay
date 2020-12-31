#include "Sphere.h"
#include "Plane.h"
#include "trace/HitPoint.h"

#include "math/Spherical.h"

#include <array>
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

void Sphere::intersects(const Ray& in, HitPoint& out) const
{
	out.resetSuccessful();

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
		out.makeSuccessful();

		// Setup UV
		Vector3f p		 = in.t(t0);
		Vector2f uv		 = project(p);
		out.Parameter[0] = uv(0);
		out.Parameter[1] = uv(1);
	}
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
	return Spherical::cartesian_from_uv(u, v);
}

Vector3f Sphere::surfacePoint(float u, float v) const
{
	return normalPoint(u, v) * mRadius;
}

Vector2f Sphere::project(const Vector3f& p) const
{
	return Spherical::uv_from_point(p);
}

void Sphere::triangulate(const Vector3f& center, float radius, uint32 stacks, uint32 slices, std::vector<float>& vertices)
{
	const uint32 count = slices * stacks;
	vertices.reserve(count * 3);

	float drho	 = 3.141592f / (float)stacks;
	float dtheta = 2 * 3.141592f / (float)slices;

	auto add = [&](const Vector3f& v) {vertices.push_back(v[0]); vertices.push_back(v[1]); vertices.push_back(v[2]); };

	for (uint32 i = 0; i < stacks; ++i) {
		float rho  = (float)i * drho;
		float srho = (float)(sin(rho));
		float crho = (float)(cos(rho));

		for (uint32 j = 0; j < slices; ++j) {
			float theta	 = (j == slices) ? 0.0f : j * dtheta;
			float stheta = (float)(-sin(theta));
			float ctheta = (float)(cos(theta));

			float x = stheta * srho;
			float y = ctheta * srho;
			float z = crho;
			add(Vector3f(x, y, z) * radius + center);
		}
	}
}

void Sphere::triangulateIndices(uint32 stacks, uint32 slices, std::vector<uint32>& indices, uint32 off)
{
	using Index = uint32;

	for (Index i = 0; i < stacks; ++i) {
		const Index currSliceOff = i * slices;
		const Index nextSliceOff = i + 1 < stacks ? (i + 1) * slices : 0;

		for (Index j = 0; j < slices; ++j) {
			const Index nextJ = j + 1 < slices ? j + 1 : 0;
			const Index id0	  = currSliceOff + j;
			const Index id1	  = currSliceOff + nextJ;
			const Index id2	  = nextSliceOff + j;
			const Index id3	  = nextSliceOff + nextJ;

			Plane::triangulateIndices({ id2 + off, id3 + off, id1 + off, id0 + off }, indices);
		}
	}
}
} // namespace PR
