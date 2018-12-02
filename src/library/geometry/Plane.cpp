#include "Plane.h"

#include "math/SIMD.h"

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

void Plane::intersects(const Ray& in, SingleCollisionOutput& out) const
{
	out.HitDistance = std::numeric_limits<float>::infinity();

	float ln = dotV(in.Direction[0], in.Direction[1], in.Direction[2],
					mNormal_Cache(0), mNormal_Cache(1), mNormal_Cache(2));

	float pn = dotV(float(mPosition(0) - in.Origin[0]),
					float(mPosition(1) - in.Origin[1]),
					float(mPosition(2) - in.Origin[2]),
					mNormal_Cache(0), mNormal_Cache(1), mNormal_Cache(2));

	if (std::abs(ln) > PR_PLANE_INTERSECT_EPSILON) {
		const float t = pn / ln;

		if (t > PR_PLANE_INTERSECT_EPSILON) {
			float px = in.Origin[0] - mPosition(0) + in.Direction[0] * t;
			float py = in.Origin[1] - mPosition(1) + in.Direction[1] * t;
			float pz = in.Origin[2] - mPosition(2) + in.Direction[2] * t;

			out.UV[0] = dotV(px, py, pz, mXAxis(0), mXAxis(1), mXAxis(2)) * mInvXLenSqr_Cache;
			out.UV[1] = dotV(px, py, pz, mYAxis(0), mYAxis(1), mYAxis(2)) * mInvYLenSqr_Cache;

			if (out.UV[0] >= 0 && out.UV[0] <= 1
				&& out.UV[1] >= 0 && out.UV[1] <= 1) {
				out.HitDistance = t;
				return;
			}
		}
	}
}

void Plane::intersects(const RayPackage& in, CollisionOutput& out) const
{
	using namespace simdpp;

	const vfloat inf = make_float(std::numeric_limits<float>::infinity());

	vfloat ln = dotV(in.Direction[0], in.Direction[1], in.Direction[2],
					 mNormal_Cache(0), mNormal_Cache(1), mNormal_Cache(2));

	vfloat pn = dotV(vfloat(mPosition(0) - in.Origin[0]),
					 vfloat(mPosition(1) - in.Origin[1]),
					 vfloat(mPosition(2) - in.Origin[2]),
					 mNormal_Cache(0), mNormal_Cache(1), mNormal_Cache(2));

	out.HitDistance = pn / ln;

	vfloat px = in.Origin[0] - mPosition(0) + in.Direction[0] * out.HitDistance;
	vfloat py = in.Origin[1] - mPosition(1) + in.Direction[1] * out.HitDistance;
	vfloat pz = in.Origin[2] - mPosition(2) + in.Direction[2] * out.HitDistance;

	out.UV[0] = dotV(px, py, pz, mXAxis(0), mXAxis(1), mXAxis(2)) * mInvXLenSqr_Cache;
	out.UV[1] = dotV(px, py, pz, mYAxis(0), mYAxis(1), mYAxis(2)) * mInvYLenSqr_Cache;

	mask_float32v succ = (out.UV[0] >= 0) & (out.UV[0] <= 1)
						 & (out.UV[1] >= 0) & (out.UV[1] <= 1)
						 & (out.HitDistance > PR_PLANE_INTERSECT_EPSILON);
	out.HitDistance = blend(out.HitDistance, inf, succ);
}

void Plane::projectV(const vfloat& px, const vfloat& py, const vfloat& pz,
					 vfloat& u, vfloat& v) const
{
	u = dotV(vfloat(px - mPosition(0)), vfloat(py - mPosition(1)), vfloat(pz - mPosition(2)),
			 mXAxis(0), mXAxis(1), mXAxis(2))
		* mInvXLenSqr_Cache;
	v = dotV(vfloat(px - mPosition(0)), vfloat(py - mPosition(1)), vfloat(pz - mPosition(2)),
			 mYAxis(0), mYAxis(1), mYAxis(2))
		* mInvYLenSqr_Cache;
}

Eigen::Vector2f Plane::project(const Eigen::Vector3f& point) const
{
	Eigen::Vector3f p = point - mPosition;
	return Eigen::Vector2f(
		mXAxis.dot(p) * mInvXLenSqr_Cache,
		mYAxis.dot(p) * mInvYLenSqr_Cache);
}
} // namespace PR
