#include "BoundingBox.h"
#include "Plane.h"

#include "ray/Ray.h"

#include <utility>

namespace PR {
BoundingBox::BoundingBox()
	: mUpperBound(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity())
	, mLowerBound(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity())
{
}

BoundingBox::BoundingBox(const Eigen::Vector3f& upperbound, const Eigen::Vector3f& lowerbound)
	: mUpperBound(upperbound.array().max(lowerbound.array()).matrix())
	, mLowerBound(lowerbound.array().min(upperbound.array()).matrix())
{
}

BoundingBox::BoundingBox(float width, float height, float depth)
	: mUpperBound(width / 2, height / 2, depth / 2)
	, mLowerBound(-width / 2, -height / 2, -depth / 2)
{
	PR_ASSERT(width >= 0, "width has to be positive");
	PR_ASSERT(height >= 0, "height has to be positive");
	PR_ASSERT(depth >= 0, "depth has to be positive");
}

void BoundingBox::inflate(float eps, bool maxDir)
{
	Eigen::Vector3f diff = (mUpperBound - mLowerBound).cwiseAbs();

	if (maxDir) {
		for (int i = 0; i < 3; ++i) {
			if (diff(i) <= eps)
				mUpperBound(i) += eps;
		}
	} else {
		for (int i = 0; i < 3; ++i) {
			if (diff(i) <= eps) {
				mLowerBound(i) -= eps;
				mUpperBound(i) += eps;
			}
		}
	}
}

BoundingBox::Intersection BoundingBox::intersects(const Ray& ray) const
{
	BoundingBox::Intersection r;

	const Eigen::Vector3f idir = ray.direction().cwiseInverse();
	const Eigen::Vector3f vmin = (lowerBound() - ray.origin()).cwiseProduct(idir);
	const Eigen::Vector3f vmax = (upperBound() - ray.origin()).cwiseProduct(idir);

	const float tmin = vmin.array().min(vmax.array()).maxCoeff();
	const float tmax = vmin.array().max(vmax.array()).minCoeff();

	r.T = tmin <= 0 ? tmax : tmin;
	if (tmax >= tmin && r.T > PR_EPSILON) {
		r.Position   = ray.origin() + ray.direction() * r.T;
		r.Successful = true;
	} else {
		r.Successful = false;
	}

	return r;
}

void BoundingBox::intersectsV(const CollisionInput& in, CollisionOutput& out) const
{
	using namespace simdpp;

	float32v entry, exit;
	for (int i = 0; i < 3; ++i) {
		const float32v vmin = (lowerBound()(i) - in.RayOrigin[i]) * in.RayInvDirection[i];
		const float32v vmax = (upperBound()(i) - in.RayOrigin[i]) * in.RayInvDirection[i];

		if (i == 0) {
			entry = min(vmin, vmax);
			exit  = max(vmin, vmax);
		} else {
			entry = max(min(vmin, vmax), entry);
			exit  = min(max(vmin, vmax), exit);
		}
	}

	const float32v inf = make_float(std::numeric_limits<float>::infinity());
	out.HitDistance	= blend(exit, entry, entry < 0);
	out.HitDistance	= blend(out.HitDistance, inf,
							   (exit >= entry) & (out.HitDistance > PR_EPSILON));
}

bool BoundingBox::intersectsSimple(const Ray& ray) const
{
	const Eigen::Vector3f idir = ray.direction().cwiseInverse();
	const Eigen::Vector3f vmin = (lowerBound() - ray.origin()).cwiseProduct(idir);
	const Eigen::Vector3f vmax = (upperBound() - ray.origin()).cwiseProduct(idir);

	const float tmin = vmin.array().min(vmax.array()).maxCoeff();
	const float tmax = vmin.array().max(vmax.array()).minCoeff();

	const float t = tmin <= 0 ? tmax : tmin;
	return tmax >= tmin && t > PR_EPSILON;
}

BoundingBox::IntersectionRange BoundingBox::intersectsRange(const Ray& ray) const
{
	BoundingBox::IntersectionRange r;

	const Eigen::Vector3f idir = ray.direction().cwiseInverse();
	const Eigen::Vector3f vmin = (lowerBound() - ray.origin()).cwiseProduct(idir);
	const Eigen::Vector3f vmax = (upperBound() - ray.origin()).cwiseProduct(idir);

	r.Entry = vmin.array().min(vmax.array()).maxCoeff();
	r.Exit  = vmin.array().max(vmax.array()).minCoeff();

	r.Successful = (r.Exit >= r.Entry && r.Exit > PR_EPSILON);

	return r;
}

BoundingBox::IntersectionRangeV BoundingBox::intersectsRangeV(const CollisionInput& in) const
{
	using namespace simdpp;

	BoundingBox::IntersectionRangeV r;

	for (int i = 0; i < 3; ++i) {
		const float32v vmin = (lowerBound()(i) - in.RayOrigin[i]) * in.RayInvDirection[i];
		const float32v vmax = (upperBound()(i) - in.RayOrigin[i]) * in.RayInvDirection[i];

		if (i == 0) {
			r.Entry = min(vmin, vmax);
			r.Exit  = max(vmin, vmax);
		} else {
			r.Entry = max(min(vmin, vmax), r.Entry);
			r.Exit  = min(max(vmin, vmax), r.Exit);
		}
	}

	r.Successful = (r.Exit >= r.Entry) & (r.Exit > PR_EPSILON);

	return r;
}

BoundingBox::FaceSide BoundingBox::getIntersectionSide(const BoundingBox::Intersection& intersection) const
{
	const Eigen::Vector3f minDist = (intersection.Position - lowerBound()).cwiseAbs();
	const Eigen::Vector3f maxDist = (intersection.Position - upperBound()).cwiseAbs();

	BoundingBox::FaceSide side = FS_Left;
	float f					   = minDist(0);

	if (maxDist(0) < f) {
		side = FS_Right;
		f	= maxDist(0);
	}

	if (minDist(1) < f) {
		side = FS_Bottom;
		f	= minDist(1);
	}

	if (maxDist(1) < f) {
		side = FS_Top;
		f	= maxDist(1);
	}

	if (minDist(2) < f) {
		side = FS_Front;
		f	= minDist(2);
	}

	if (maxDist(2) < f) {
		side = FS_Back;
	}

	return side;
}

Plane BoundingBox::getFace(FaceSide side) const
{
	const Eigen::Vector3f diff = upperBound() - lowerBound();

	switch (side) {
	default:
	case FS_Front:
		return Plane(lowerBound(),
					 Eigen::Vector3f(diff(0), 0, 0),
					 Eigen::Vector3f(0, diff(1), 0));
	case FS_Back:
		return Plane(Eigen::Vector3f(upperBound()(0), lowerBound()(1), upperBound()(2)),
					 Eigen::Vector3f(-diff(0), 0, 0),
					 Eigen::Vector3f(0, diff(1), 0));
	case FS_Left:
		return Plane(Eigen::Vector3f(lowerBound()(0), lowerBound()(1), upperBound()(2)),
					 Eigen::Vector3f(0, 0, -diff(2)),
					 Eigen::Vector3f(0, diff(1), 0));
	case FS_Right:
		return Plane(Eigen::Vector3f(upperBound()(0), lowerBound()(1), lowerBound()(2)),
					 Eigen::Vector3f(0, 0, diff(2)),
					 Eigen::Vector3f(0, diff(1), 0));
	case FS_Top:
		return Plane(Eigen::Vector3f(lowerBound()(0), upperBound()(1), lowerBound()(2)),
					 Eigen::Vector3f(diff(0), 0, 0),
					 Eigen::Vector3f(0, 0, diff(2)));
	case FS_Bottom:
		return Plane(Eigen::Vector3f(upperBound()(0), lowerBound()(1), lowerBound()(2)),
					 Eigen::Vector3f(-diff(0), 0, 0),
					 Eigen::Vector3f(0, 0, diff(2)));
	}
}

void BoundingBox::combine(const Eigen::Vector3f& point)
{
	mUpperBound = mUpperBound.array().max(point.array()).matrix();
	mLowerBound = mLowerBound.array().min(point.array()).matrix();
}

void BoundingBox::combine(const BoundingBox& other)
{
	combine(other.lowerBound());
	combine(other.upperBound());
}
} // namespace PR
