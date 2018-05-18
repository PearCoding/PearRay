#include "Plane.h"

#include "ray/Ray.h"

namespace PR {
#define PR_PLANE_INTERSECT_EPSILON (PR_EPSILON)
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

Plane::Plane(const Eigen::Vector3f& pos, const Eigen::Vector3f& xAxis, const Eigen::Vector3f& yAxis)
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
	mNormal_Cache	  = mXAxis.cross(mYAxis);
	mSurfaceArea_Cache = mNormal_Cache.norm();
	mNormal_Cache.normalize();

	// Div Zero?
	mInvXLenSqr_Cache = 1 / mXAxis.squaredNorm();
	mInvYLenSqr_Cache = 1 / mYAxis.squaredNorm();
}

void Plane::setXAxis(const Eigen::Vector3f& v)
{
	mXAxis = v;
	recache();
}

void Plane::setYAxis(const Eigen::Vector3f& v)
{
	mYAxis = v;
	recache();
}

void Plane::setAxis(const Eigen::Vector3f& xAxis, const Eigen::Vector3f& yAxis)
{
	mXAxis = xAxis;
	mYAxis = yAxis;
	recache();
}

BoundingBox Plane::toLocalBoundingBox() const
{
	

	BoundingBox box(mXAxis + mYAxis, Eigen::Vector3f(0, 0, 0));
	box.inflate(EPSILON_BOUND);
	return box;
}

bool Plane::contains(const Eigen::Vector3f& point) const
{
	

	Eigen::Vector3f p = point - mPosition;
	if (p.dot(mNormal_Cache) <= std::numeric_limits<float>::epsilon()) // Is on the plane
	{
		float u = mXAxis.dot(p) * mInvXLenSqr_Cache;
		float v = mYAxis.dot(p) * mInvYLenSqr_Cache;

		if (v >= 0 && v <= 1 && u >= 0 && u <= 1)
			return true;
	}
	return false;
}

Plane::Intersection Plane::intersects(const Ray& ray) const
{
	

	Plane::Intersection r;
	float ln = ray.direction().dot(mNormal_Cache);
	float pn = (mPosition - ray.origin()).dot(mNormal_Cache);

	r.Successful = false;
	if (std::abs(ln) <= PR_PLANE_INTERSECT_EPSILON) // Parallel or on the plane
	{
		return r;
	} else {
		r.T = pn / ln;

		if (r.T < PR_PLANE_INTERSECT_EPSILON) {
			return r;
		} else {
			r.Position = ray.origin() + ray.direction() * r.T;
			r.UV	= project(r.Position);

			if (r.UV.x() >= 0 && r.UV.x() <= 1 && r.UV.y() >= 0 && r.UV.y() <= 1) {
				r.Successful = true;
				return r;
			}
		}
		return r;
	}
}

Eigen::Vector2f Plane::project(const Eigen::Vector3f& point) const
{
	

	Eigen::Vector3f p = point - mPosition;
	return Eigen::Vector2f(
		mXAxis.dot(p) * mInvXLenSqr_Cache,
		mYAxis.dot(p) * mInvYLenSqr_Cache);
}
}
