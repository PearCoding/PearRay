#include "BoundingBox.h"
#include "trace/HitPoint.h"
#include "Plane.h"

namespace PR {
BoundingBox::BoundingBox()
	: mUpperBound(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity())
	, mLowerBound(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity())
{
}

BoundingBox::BoundingBox(const Vector3f& upperbound, const Vector3f& lowerbound)
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
	Vector3f diff = (mUpperBound - mLowerBound).cwiseAbs();

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

void BoundingBox::intersects(const Ray& in, HitPoint& out) const
{
	out.HitDistance		  = std::numeric_limits<float>::infinity();
	const Vector3f invDir = in.Direction.cwiseInverse();

	float entry = -std::numeric_limits<float>::infinity();
	float exit	= std::numeric_limits<float>::infinity();
	for (int i = 0; i < 3; ++i) {
		const float vmin = (lowerBound()(i) - in.Origin[i]) * invDir[i];
		const float vmax = (upperBound()(i) - in.Origin[i]) * invDir[i];

		entry = std::max(std::min(vmin, vmax), entry);
		exit  = std::min(std::max(vmin, vmax), exit);
	}

	float minE = entry <= 0 ? exit : entry;
	if (exit >= entry) {
		if (in.isInsideRange(minE))
			out.HitDistance = minE;
		else if (in.isInsideRange(exit))
			out.HitDistance = exit;
	}
}

BoundingBox::IntersectionRange BoundingBox::intersectsRange(const Ray& in) const
{
	BoundingBox::IntersectionRange r;
	const Vector3f invDir = in.Direction.cwiseInverse();

	// Leading loop part
	{
		const float vmin = (lowerBound()(0) - in.Origin[0]) * invDir[0];
		const float vmax = (upperBound()(0) - in.Origin[0]) * invDir[0];
		r.Entry			 = std::min(vmin, vmax);
		r.Exit			 = std::max(vmin, vmax);
	}

	PR_UNROLL_LOOP(2)
	for (int i = 1; i < 3; ++i) {
		const float vmin = (lowerBound()(i) - in.Origin[i]) * invDir[i];
		const float vmax = (upperBound()(i) - in.Origin[i]) * invDir[i];

		r.Entry = std::max(std::min(vmin, vmax), r.Entry);
		r.Exit	= std::min(std::max(vmin, vmax), r.Exit);
	}

	r.Entry		 = std::max(in.MinT, r.Entry);
	r.Exit		 = std::min(in.MaxT, r.Exit);
	r.Successful = (r.Exit >= r.Entry && in.isInsideRange(r.Exit));

	return r;
}

BoundingBox::FaceSide BoundingBox::getIntersectionSide(const Vector3f& intersection) const
{
	const Vector3f minDist = (intersection - lowerBound()).cwiseAbs();
	const Vector3f maxDist = (intersection - upperBound()).cwiseAbs();

	BoundingBox::FaceSide side = FS_Left;
	float f					   = minDist(0);

	if (maxDist(0) < f) {
		side = FS_Right;
		f	 = maxDist(0);
	}

	if (minDist(1) < f) {
		side = FS_Bottom;
		f	 = minDist(1);
	}

	if (maxDist(1) < f) {
		side = FS_Top;
		f	 = maxDist(1);
	}

	if (minDist(2) < f) {
		side = FS_Front;
		f	 = minDist(2);
	}

	if (maxDist(2) < f) {
		side = FS_Back;
	}

	return side;
}

Plane BoundingBox::getFace(FaceSide side) const
{
	const Vector3f diff = upperBound() - lowerBound();

	switch (side) {
	default:
	case FS_Front:
		return Plane(lowerBound(),
					 Vector3f(diff(0), 0, 0),
					 Vector3f(0, diff(1), 0));
	case FS_Back:
		return Plane(Vector3f(upperBound()(0), lowerBound()(1), upperBound()(2)),
					 Vector3f(-diff(0), 0, 0),
					 Vector3f(0, diff(1), 0));
	case FS_Left:
		return Plane(Vector3f(lowerBound()(0), lowerBound()(1), upperBound()(2)),
					 Vector3f(0, 0, -diff(2)),
					 Vector3f(0, diff(1), 0));
	case FS_Right:
		return Plane(Vector3f(upperBound()(0), lowerBound()(1), lowerBound()(2)),
					 Vector3f(0, 0, diff(2)),
					 Vector3f(0, diff(1), 0));
	case FS_Top:
		return Plane(Vector3f(lowerBound()(0), upperBound()(1), lowerBound()(2)),
					 Vector3f(diff(0), 0, 0),
					 Vector3f(0, 0, diff(2)));
	case FS_Bottom:
		return Plane(Vector3f(upperBound()(0), lowerBound()(1), lowerBound()(2)),
					 Vector3f(-diff(0), 0, 0),
					 Vector3f(0, 0, diff(2)));
	}
}

void BoundingBox::combine(const Vector3f& point)
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
