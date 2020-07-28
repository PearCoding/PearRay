#include "BoundingBox.h"
#include "Plane.h"
#include "trace/HitPoint.h"

namespace PR {
BoundingBox::BoundingBox()
	: mUpperBound(-PR_INF, -PR_INF, -PR_INF)
	, mLowerBound(PR_INF, PR_INF, PR_INF)
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
	out.HitDistance		  = PR_INF;
	const Vector3f invDir = in.Direction.cwiseInverse();
	const Vector3f vmin	  = invDir.cwiseProduct(lowerBound() - in.Origin);
	const Vector3f vmax	  = invDir.cwiseProduct(upperBound() - in.Origin);

	float entry = -PR_INF;
	float exit	= PR_INF;
	for (int i = 0; i < 3; ++i) {
		entry = std::max(std::min(vmin[i], vmax[i]), entry);
		exit  = std::min(std::max(vmin[i], vmax[i]), exit);
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
	const Vector3f vmin	  = invDir.cwiseProduct(lowerBound() - in.Origin);
	const Vector3f vmax	  = invDir.cwiseProduct(upperBound() - in.Origin);

	r.Entry = std::min(vmin[0], vmax[0]);
	r.Exit	= std::max(vmin[0], vmax[0]);
	PR_UNROLL_LOOP(2)
	for (int i = 1; i < 3; ++i) {
		r.Entry = std::max(std::min(vmin[i], vmax[i]), r.Entry);
		r.Exit	= std::min(std::max(vmin[i], vmax[i]), r.Exit);
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
