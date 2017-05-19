#include "Plane.h"

#include "ray/Ray.h"

#include "performance/Performance.h"

namespace PR
{
	#define PR_PLANE_INTERSECT_EPSILON (PR_EPSILON)
	constexpr float EPSILON_BOUND = 0.00001f;

	Plane::Plane() :
		mPosition(0, 0, 0), mXAxis(1, 0, 0), mYAxis(0, 1, 0),
		mNormal_Cache(0,0,1), mSurfaceArea_Cache(1),
		mInvXLenSqr_Cache(1), mInvYLenSqr_Cache(1)
	{
	}

	Plane::Plane(const Eigen::Vector3f& pos, const Eigen::Vector3f& xAxis, const Eigen::Vector3f& yAxis) :
		mPosition(pos), mXAxis(xAxis), mYAxis(yAxis)
	{
		recache();
	}

	Plane::Plane(float width, float height) :
		mPosition(0,0,0),
		mXAxis(width, 0, 0),
		mYAxis(0, height, 0),
		mNormal_Cache(0, 0, 1),
		mSurfaceArea_Cache(width * height),
		mInvXLenSqr_Cache(1/(width*width)),
		mInvYLenSqr_Cache(1/(height*height))
	{
		PR_ASSERT(width > 0, "width has to be greater than 0");
		PR_ASSERT(height > 0, "height has to be greater than 0");
	}

	void Plane::recache()
	{
		mNormal_Cache = mXAxis.cross(mYAxis);
		mSurfaceArea_Cache = mNormal_Cache.norm();
		mNormal_Cache.normalize();

		//Div Zero?
		mInvXLenSqr_Cache = 1/mXAxis.squaredNorm();
		mInvYLenSqr_Cache = 1/mYAxis.squaredNorm();
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
		PR_GUARD_PROFILE();

		Eigen::Vector3f diff = (mXAxis - mYAxis).cwiseAbs();
		Eigen::Vector3f min(0,0,0), max(0,0,0);

		for(int i = 0; i < 3; i++)
		{
			if(diff(i) <= PR_EPSILON)
			{
				min(i) = -EPSILON_BOUND;
				max(i) = EPSILON_BOUND;
			}
			else
			{
				if(mXAxis(i)>mYAxis(i))
				{
					min(i) = mYAxis(i);
					max(i) = mXAxis(i);
				}
				else
				{
					min(i) = mXAxis(i);
					max(i) = mYAxis(i);
				}
			}
		}

		return BoundingBox(max, min);
	}

	bool Plane::contains(const Eigen::Vector3f& point) const
	{
		PR_GUARD_PROFILE();

		Eigen::Vector3f p = point-mPosition;
		if (p.dot(mNormal_Cache) <= std::numeric_limits<float>::epsilon())// Is on the plane
		{
			float u = mXAxis.dot(p) * mInvXLenSqr_Cache;
			float v = mYAxis.dot(p) * mInvYLenSqr_Cache;

			if (v >= 0 && v <= 1 && u >= 0 && u <= 1)
				return true;
		}
		return false;
	}

	bool Plane::intersects(const Ray& ray, Eigen::Vector3f& collisionPoint, float& t, float& u, float& v) const
	{
		PR_GUARD_PROFILE();

		float ln = ray.direction().dot(mNormal_Cache);
		float pn = (mPosition - ray.startPosition()).dot(mNormal_Cache);

		if (std::abs(ln) <= PR_PLANE_INTERSECT_EPSILON)//Parallel or on the plane
		{
			return false;
			//if (pn <= std::numeric_limits<float>::epsilon())// Is on the plane!
			//{
			//	// TODO: Should we make this case special?
			//}
		}
		else
		{
			t = pn / ln;

			if (t < PR_PLANE_INTERSECT_EPSILON)
			{
				return false;
			}
			else
			{
				collisionPoint = ray.startPosition() + ray.direction() * t;
				Eigen::Vector3f p = collisionPoint - mPosition;
				u = mXAxis.dot(p) * mInvXLenSqr_Cache;
				v = mYAxis.dot(p) * mInvYLenSqr_Cache;

				if (v >= 0 && v <= 1 && u >= 0 && u <= 1)
					return true;
			}
			return false;
		}
	}

	void Plane::project(const Eigen::Vector3f& point, float& u, float& v) const
	{
		PR_GUARD_PROFILE();

		Eigen::Vector3f p = point-mPosition;
		u = mXAxis.dot(p) * mInvXLenSqr_Cache;
		v = mYAxis.dot(p) * mInvYLenSqr_Cache;
	}
}
