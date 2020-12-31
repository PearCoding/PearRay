#include "BoundingBox.h"
#include "Plane.h"
#include "trace/HitPoint.h"

#include <array>

namespace PR {
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

	FaceSide side = FaceSide::Left;
	float f		  = minDist(0);

	if (maxDist(0) < f) {
		side = FaceSide::Right;
		f	 = maxDist(0);
	}

	if (minDist(1) < f) {
		side = FaceSide::Bottom;
		f	 = minDist(1);
	}

	if (maxDist(1) < f) {
		side = FaceSide::Top;
		f	 = maxDist(1);
	}

	if (minDist(2) < f) {
		side = FaceSide::Front;
		f	 = minDist(2);
	}

	if (maxDist(2) < f) {
		side = FaceSide::Back;
	}

	return side;
}

Plane BoundingBox::getFace(FaceSide side) const
{
	const Vector3f diff = upperBound() - lowerBound();

	switch (side) {
	default:
	case FaceSide::Front:
		return Plane(lowerBound(),
					 Vector3f(diff(0), 0, 0),
					 Vector3f(0, diff(1), 0));
	case FaceSide::Back:
		return Plane(Vector3f(upperBound()(0), lowerBound()(1), upperBound()(2)),
					 Vector3f(-diff(0), 0, 0),
					 Vector3f(0, diff(1), 0));
	case FaceSide::Left:
		return Plane(Vector3f(lowerBound()(0), lowerBound()(1), upperBound()(2)),
					 Vector3f(0, 0, -diff(2)),
					 Vector3f(0, diff(1), 0));
	case FaceSide::Right:
		return Plane(Vector3f(upperBound()(0), lowerBound()(1), lowerBound()(2)),
					 Vector3f(0, 0, diff(2)),
					 Vector3f(0, diff(1), 0));
	case FaceSide::Top:
		return Plane(Vector3f(lowerBound()(0), upperBound()(1), lowerBound()(2)),
					 Vector3f(diff(0), 0, 0),
					 Vector3f(0, 0, diff(2)));
	case FaceSide::Bottom:
		return Plane(Vector3f(upperBound()(0), lowerBound()(1), lowerBound()(2)),
					 Vector3f(-diff(0), 0, 0),
					 Vector3f(0, 0, diff(2)));
	}
}

Vector3f BoundingBox::corner(int n) const
{
	switch (n) {
	default:
	case 0:
		return lowerBound();
	case 1:
		return Vector3f(upperBound().x(), lowerBound().y(), lowerBound().z());
	case 2:
		return Vector3f(lowerBound().x(), upperBound().y(), lowerBound().z());
	case 3:
		return Vector3f(upperBound().x(), upperBound().y(), lowerBound().z());
	case 4:
		return Vector3f(lowerBound().x(), lowerBound().y(), upperBound().z());
	case 5:
		return Vector3f(upperBound().x(), lowerBound().y(), upperBound().z());
	case 6:
		return Vector3f(lowerBound().x(), upperBound().y(), upperBound().z());
	case 7:
		return upperBound();
	}
}

static std::array<int, 4> INDICES[6] = {
	{ 1, 0, 2, 3 }, // Bottom
	{ 3, 7, 5, 1 }, // Right
	{ 7, 6, 4, 5 }, // Top
	{ 2, 0, 4, 6 }, // Left
	{ 6, 7, 3, 2 }, // Back
	{ 0, 1, 5, 4 }, // Front
};
void BoundingBox::triangulateIndices(const std::array<uint32, 8>& corners, std::vector<uint32>& indices)
{
	for (int i = 0; i < 6; ++i)
		Plane::triangulateIndices({ corners[INDICES[i][0]], corners[INDICES[i][1]], corners[INDICES[i][2]], corners[INDICES[i][3]] }, indices);
}
} // namespace PR
