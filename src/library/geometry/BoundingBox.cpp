#include "BoundingBox.h"
#include "Plane.h"

#include "ray/Ray.h"

#include <utility>

#include "performance/Performance.h"

namespace PR
{
	BoundingBox::BoundingBox() :
		 mBox()
	{
	}

	BoundingBox::BoundingBox(const Eigen::Vector3f& upperbound, const Eigen::Vector3f& lowerbound):
		 mBox(lowerbound, upperbound)
	{
	}

	BoundingBox::BoundingBox(float width, float height, float depth) :
		mBox(Eigen::Vector3f(-width/2, -height/2, -depth/2),
			Eigen::Vector3f(width/2, height/2, depth/2))
	{
		PR_ASSERT(width > PR_EPSILON, "width has to be greater than 0");
		PR_ASSERT(height > PR_EPSILON, "height has to be greater than 0");
		PR_ASSERT(depth > PR_EPSILON, "depth has to be greater than 0");
	}

	bool BoundingBox::intersects(const Ray& ray, float& t) const
	{
		PR_GUARD_PROFILE();

		const Eigen::Vector3f idir = ray.direction().cwiseInverse();
		const Eigen::Vector3f vmin = (lowerBound() - ray.startPosition()).cwiseProduct(idir);
		const Eigen::Vector3f vmax = (upperBound() - ray.startPosition()).cwiseProduct(idir);

		const float tmin = vmin.array().min(vmax.array()).maxCoeff();
		const float tmax = vmin.array().max(vmax.array()).minCoeff();

		t = tmin <= 0 ? tmax : tmin;
		return tmax >= tmin && t > PR_EPSILON;
	}

	bool BoundingBox::intersects(const Ray& ray, Eigen::Vector3f& collisionPoint, float& t) const
	{
		PR_GUARD_PROFILE();

		const Eigen::Vector3f idir = ray.direction().cwiseInverse();
		const Eigen::Vector3f vmin = (lowerBound() - ray.startPosition()).cwiseProduct(idir);
		const Eigen::Vector3f vmax = (upperBound() - ray.startPosition()).cwiseProduct(idir);

		const float tmin = vmin.array().min(vmax.array()).maxCoeff();
		const float tmax = vmin.array().max(vmax.array()).minCoeff();

		t = tmin <= 0 ? tmax : tmin;
		if (tmax >= tmin && t > PR_EPSILON)
		{
			collisionPoint = ray.startPosition() + ray.direction() * t;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool BoundingBox::intersects(const Ray& ray, Eigen::Vector3f& collisionPoint, float& t, FaceSide& side) const
	{
		PR_GUARD_PROFILE();

		if (!intersects(ray, collisionPoint, t))
			return false;

		Eigen::Vector3f minDist = (collisionPoint - lowerBound()).cwiseAbs();
		Eigen::Vector3f maxDist = (collisionPoint - upperBound()).cwiseAbs();

		side = FS_Left;
		float f = minDist(0);

		if (maxDist(0) < f)
		{
			side = FS_Right;
			f = maxDist(0);
		}

		if (minDist(1) < f)
		{
			side = FS_Bottom;
			f = minDist(1);
		}

		if (maxDist(1) < f)
		{
			side = FS_Top;
			f = maxDist(1);
		}

		if (minDist(2) < f)
		{
			side = FS_Front;
			f = minDist(2);
		}

		if (maxDist(2) < f)
		{
			side = FS_Back;
			//f = PM::pm_GetZ(maxDist);
		}

		return true;
	}

	Plane BoundingBox::getFace(FaceSide side) const
	{
		PR_GUARD_PROFILE();

		Eigen::Vector3f diff = upperBound() - lowerBound();

		switch (side)
		{
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
				Eigen::Vector3f(diff(0), 0,0),
				Eigen::Vector3f(0, 0, diff(2)));
		case FS_Bottom:
			return Plane(Eigen::Vector3f(upperBound()(0), lowerBound()(1), lowerBound()(2)),
				Eigen::Vector3f(-diff(0), 0,0),
				Eigen::Vector3f(0, 0, diff(2)));
		}
	}
}
