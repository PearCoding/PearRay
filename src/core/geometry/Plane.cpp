#include "Plane.h"
#include "trace/HitPoint.h"

namespace PR {
constexpr float EPSILON_BOUND = 0.0001f;

Plane::Plane()
	: mPosition(0, 0, 0)
	, mXAxis(1, 0, 0)
	, mYAxis(0, 1, 0)
	, mNormal_Cache(0, 0, 1)
	, mSurfaceArea_Cache(1)
	, mInvXLenSqr_Cache(1)
	, mInvYLenSqr_Cache(1)
{
}

Plane::Plane(const Vector3f& pos, const Vector3f& xAxis, const Vector3f& yAxis)
	: mPosition(pos)
	, mXAxis(xAxis)
	, mYAxis(yAxis)
{
	recache();
}

Plane::Plane(float width, float height)
	: mPosition(0, 0, 0)
	, mXAxis(width, 0, 0)
	, mYAxis(0, height, 0)
	, mNormal_Cache(0, 0, 1)
	, mSurfaceArea_Cache(width * height)
	, mInvXLenSqr_Cache(1 / (width * width))
	, mInvYLenSqr_Cache(1 / (height * height))
{
	PR_ASSERT(width > 0, "width has to be greater than 0");
	PR_ASSERT(height > 0, "height has to be greater than 0");
}

void Plane::recache()
{
	mNormal_Cache	   = mXAxis.cross(mYAxis);
	mSurfaceArea_Cache = mNormal_Cache.norm();
	mNormal_Cache.normalize();

	// Div Zero?
	mInvXLenSqr_Cache = 1 / mXAxis.squaredNorm();
	mInvYLenSqr_Cache = 1 / mYAxis.squaredNorm();
}

void Plane::setXAxis(const Vector3f& v)
{
	mXAxis = v;
	recache();
}

void Plane::setYAxis(const Vector3f& v)
{
	mYAxis = v;
	recache();
}

void Plane::setAxis(const Vector3f& xAxis, const Vector3f& yAxis)
{
	mXAxis = xAxis;
	mYAxis = yAxis;
	recache();
}

BoundingBox Plane::toLocalBoundingBox() const
{
	BoundingBox box(mXAxis + mYAxis, Vector3f(0, 0, 0));
	box.inflate(EPSILON_BOUND);
	return box;
}

bool Plane::contains(const Vector3f& point) const
{
	Vector3f p = point - mPosition;
	if (p.dot(mNormal_Cache) <= std::numeric_limits<float>::epsilon()) // Is on the plane
	{
		float u = mXAxis.dot(p) * mInvXLenSqr_Cache;
		float v = mYAxis.dot(p) * mInvYLenSqr_Cache;

		if (v >= 0 && v <= 1 && u >= 0 && u <= 1)
			return true;
	}
	return false;
}

void Plane::intersects(const Ray& in, HitPoint& out) const
{
	out.resetSuccessful();

	float ln = in.Direction.dot(mNormal_Cache);
	float pn = (mPosition - in.Origin).dot(mNormal_Cache);

	if (std::abs(ln) > PR_EPSILON) {
		const float t = pn / ln;

		if (in.isInsideRange(t)) {
			const Vector3f p = in.t(t) - mPosition;

			out.Parameter[0] = p.dot(mXAxis) * mInvXLenSqr_Cache;
			out.Parameter[1] = p.dot(mYAxis) * mInvYLenSqr_Cache;

			if (out.Parameter[0] >= 0 && out.Parameter[0] <= 1
				&& out.Parameter[1] >= 0 && out.Parameter[1] <= 1) {
				out.HitDistance = t;
				out.makeSuccessful();
				return;
			}
		}
	}
}

Vector2f Plane::project(const Vector3f& point) const
{
	Vector3f p = point - mPosition;
	return Vector2f(
		mXAxis.dot(p) * mInvXLenSqr_Cache,
		mYAxis.dot(p) * mInvYLenSqr_Cache);
}

} // namespace PR
