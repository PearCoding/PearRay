#include "Plane.h"

#include "ray/Ray.h"

#include "performance/Performance.h"

namespace PR
{
	#define PR_PLANE_INTERSECT_EPSILON (PM_EPSILON)
	constexpr float EPSILON_BOUND = 0.00001f;

	Plane::Plane() :
		mPosition(PM::pm_Set(0, 0, 0, 1)), mXAxis(PM::pm_Set(1, 0, 0, 0)), mYAxis(PM::pm_Set(0, 1, 0, 0)),
		mNormal_Cache(PM::pm_Set(0,0,1,0)), mSurfaceArea_Cache(1),
		mInvXLenSqr_Cache(1), mInvYLenSqr_Cache(1)
	{
	}

	Plane::Plane(const PM::vec3& pos, const PM::vec3& xAxis, const PM::vec3& yAxis) :
		mPosition(pos), mXAxis(xAxis), mYAxis(yAxis)
	{
		recache();
	}

	Plane::Plane(float width, float height) :
		mPosition(PM::pm_Set(0,0,0,1)),
		mXAxis(PM::pm_Set(width, 0, 0, 0)),
		mYAxis(PM::pm_Set(0, height, 0, 0)),
		mNormal_Cache(PM::pm_Set(0, 0, 1, 0)),
		mSurfaceArea_Cache(width * height),
		mInvXLenSqr_Cache(1/(width*width)),
		mInvYLenSqr_Cache(1/(height*height))
	{
		PR_ASSERT(width > 0);
		PR_ASSERT(height > 0);
	}

	void Plane::recache()
	{
		mNormal_Cache = PM::pm_Cross3D(mXAxis, mYAxis);
		mSurfaceArea_Cache = PM::pm_Magnitude3D(mNormal_Cache);
		mNormal_Cache = PM::pm_Scale(mNormal_Cache, 1/mSurfaceArea_Cache);

		//Div Zero?
		mInvXLenSqr_Cache = 1/PM::pm_MagnitudeSqr3D(mXAxis);
		mInvYLenSqr_Cache = 1/PM::pm_MagnitudeSqr3D(mYAxis);
	}

	void Plane::setXAxis(const PM::vec3& v)
	{
		mXAxis = v;
		recache();
	}

	void Plane::setYAxis(const PM::vec3& v)
	{
		mYAxis = v;
		recache();
	}

	void Plane::setAxis(const PM::vec3& xAxis, const PM::vec3& yAxis)
	{
		mXAxis = xAxis;
		mYAxis = yAxis;
		recache();
	}

	BoundingBox Plane::toLocalBoundingBox() const
	{
		PR_GUARD_PROFILE();

		BoundingBox box(PM::pm_SetW(PM::pm_Add(mXAxis, mYAxis),1), PM::pm_Set(0, 0, 0, 1));
		PM::vec3 diff = PM::pm_Abs(PM::pm_Subtract(mXAxis, mYAxis));

		if (PM::pm_GetX(diff) <= PM_EPSILON)
		{
			if (PM::pm_GetY(diff) <= PM_EPSILON)
			{
				box.setUpperBound(PM::pm_Add(box.upperBound(), PM::pm_Set(0, EPSILON_BOUND, 0)));
			}
			else if (PM::pm_GetZ(diff) <= PM_EPSILON)
			{
				box.setUpperBound(PM::pm_Add(box.upperBound(), PM::pm_Set(0, 0, EPSILON_BOUND)));
			}
		}
		else if (PM::pm_GetY(diff) <= PM_EPSILON)
		{
			if (PM::pm_GetX(diff) <= PM_EPSILON)
			{
				box.setUpperBound(PM::pm_Add(box.upperBound(), PM::pm_Set(EPSILON_BOUND, 0, 0)));
			}
			else if (PM::pm_GetZ(diff) <= PM_EPSILON)
			{
				box.setUpperBound(PM::pm_Add(box.upperBound(), PM::pm_Set(0, 0, EPSILON_BOUND)));
			}
		}
		else if (PM::pm_GetZ(diff) <= PM_EPSILON)
		{
			if (PM::pm_GetX(diff) <= PM_EPSILON)
			{
				box.setUpperBound(PM::pm_Add(box.upperBound(), PM::pm_Set(EPSILON_BOUND, 0, 0)));
			}
			else if (PM::pm_GetY(diff) <= PM_EPSILON)
			{
				box.setUpperBound(PM::pm_Add(box.upperBound(), PM::pm_Set(0, EPSILON_BOUND, 0)));
			}
		}

		return box;
	}

	bool Plane::contains(const PM::vec3& point) const
	{
		PR_GUARD_PROFILE();

		PM::vec3 p = PM::pm_Subtract(point, mPosition);
		if (PM::pm_Dot3D(p, mNormal_Cache) <= std::numeric_limits<float>::epsilon())// Is on the plane
		{			
			float u = PM::pm_Dot3D(mXAxis, p) * mInvXLenSqr_Cache;
			float v = PM::pm_Dot3D(mYAxis, p) * mInvYLenSqr_Cache;

			if (v >= 0 && v <= 1 && u >= 0 && u <= 1)
				return true;
		}
		return false;
	}

	bool Plane::intersects(const Ray& ray, PM::vec3& collisionPoint, float& t, float& u, float& v) const
	{
		PR_GUARD_PROFILE();

		float ln = PM::pm_Dot3D(ray.direction(), mNormal_Cache);
		float pn = PM::pm_Dot3D(PM::pm_Subtract(mPosition, ray.startPosition()), mNormal_Cache);

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
				collisionPoint = PM::pm_Add(ray.startPosition(), PM::pm_Scale(ray.direction(), t));
				PM::vec3 p = PM::pm_Subtract(collisionPoint, mPosition);
				u = PM::pm_Dot3D(mXAxis, p) * mInvXLenSqr_Cache;
				v = PM::pm_Dot3D(mYAxis, p) * mInvYLenSqr_Cache;

				if (v >= 0 && v <= 1 && u >= 0 && u <= 1)
					return true;
			}
			return false;
		}
	}

	void Plane::project(const PM::vec3& point, float& u, float& v) const
	{
		PR_GUARD_PROFILE();
		
		PM::vec3 p = PM::pm_Subtract(point, mPosition);
		u = PM::pm_Dot3D(mXAxis, p) * mInvXLenSqr_Cache;
		v = PM::pm_Dot3D(mYAxis, p) * mInvYLenSqr_Cache;
	}
}