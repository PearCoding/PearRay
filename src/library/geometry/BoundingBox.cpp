#include "BoundingBox.h"
#include "Plane.h"

#include "ray/Ray.h"

#include <utility>

#include "performance/Performance.h"

namespace PR
{
	BoundingBox::BoundingBox() :
		mUpperBound(PM::pm_Set(0, 0, 0, 1)), mLowerBound(PM::pm_Set(0, 0, 0, 1))
	{
	}

	constexpr float BIAS = 0.00001f;
	BoundingBox::BoundingBox(const PM::vec3& upperbound, const PM::vec3& lowerbound) :
		mUpperBound(upperbound), mLowerBound(lowerbound)
	{
		PR_GUARD_PROFILE();

		for (int i = 0; i < 3; ++i)
		{
			float ui = PM::pm_GetIndex(mUpperBound, i);
			float li = PM::pm_GetIndex(mLowerBound, i);
			float d = ui - li;

			if (std::signbit(d))//Negative
			{
				mUpperBound = PM::pm_SetIndex(mUpperBound, i, li);
				mLowerBound = PM::pm_SetIndex(mLowerBound, i, ui);
			}

			/*if (std::abs(d) < PM_EPSILON)
			{
				mUpperBound = PM::pm_SetIndex(mUpperBound, i, ui + BIAS);
			}*/
		}
	}

	BoundingBox::BoundingBox(float width, float height, float depth) :
		mUpperBound(PM::pm_Set(width/2, height/2, depth/2, 1)),
		mLowerBound(PM::pm_Set(-width/2, -height/2, -depth/2, 1))
	{
		PR_ASSERT(width > PM_EPSILON);
		PR_ASSERT(height > PM_EPSILON);
		PR_ASSERT(depth > PM_EPSILON);
	}

	BoundingBox::BoundingBox(const BoundingBox& other)
	{
		PM::pm_Copy(mUpperBound, other.upperBound());
		PM::pm_Copy(mLowerBound, other.lowerBound());
	}

	BoundingBox& BoundingBox::operator = (const BoundingBox& other)
	{
		PM::pm_Copy(mUpperBound, other.upperBound());
		PM::pm_Copy(mLowerBound, other.lowerBound());

		return *this;
	}

	bool BoundingBox::intersects(const Ray& ray, PM::vec3& collisionPoint, float& t) const
	{
		PR_GUARD_PROFILE();

		PM::vec3 vmin = PM::pm_Divide(PM::pm_Subtract(mLowerBound, ray.startPosition()),
						PM::pm_SetW(ray.direction(), 1));
		PM::vec3 vmax = PM::pm_Divide(PM::pm_Subtract(mUpperBound, ray.startPosition()),
						PM::pm_SetW(ray.direction(), 1));

		float tmin = PM::pm_MaxElement3D(PM::pm_Min(vmin, vmax));
		float tmax = PM::pm_MinElement3D(PM::pm_Max(vmin, vmax));

		t = tmin <= 0 ? tmax : tmin;
		if (tmax >= 0 && tmax >= tmin && t > PM_EPSILON)
		{
			collisionPoint = PM::pm_Add(ray.startPosition(), PM::pm_Scale(ray.direction(), t));
			return true;
		}
		else
		{
			return false;
		}
	}


	bool BoundingBox::intersects(const Ray& ray, PM::vec3& collisionPoint, float& t, FaceSide& side) const
	{
		PR_GUARD_PROFILE();

		if (!intersects(ray, collisionPoint, t))
			return false;

		PM::vec3 minDist = PM::pm_Abs(PM::pm_Subtract(collisionPoint, mLowerBound));
		PM::vec3 maxDist = PM::pm_Abs(PM::pm_Subtract(collisionPoint, mUpperBound));

		side = FS_Left;
		float f = PM::pm_GetX(minDist);

		if (PM::pm_GetX(maxDist) < f)
		{
			side = FS_Right;
			f = PM::pm_GetX(maxDist);
		}

		if (PM::pm_GetY(minDist) < f)
		{
			side = FS_Bottom;
			f = PM::pm_GetY(minDist);
		}

		if (PM::pm_GetY(maxDist) < f)
		{
			side = FS_Top;
			f = PM::pm_GetY(maxDist);
		}

		if (PM::pm_GetZ(minDist) < f)
		{
			side = FS_Front;
			f = PM::pm_GetZ(minDist);
		}

		if (PM::pm_GetZ(maxDist) < f)
		{
			side = FS_Back;
			//f = PM::pm_GetZ(maxDist);
		}

		return true;
	}

	void BoundingBox::put(const PM::vec3& point)
	{
		PR_GUARD_PROFILE();

		for (int i = 0; i < 3; ++i)
		{
			if (PM::pm_GetIndex(point, i) > PM::pm_GetIndex(mUpperBound, i))
			{
				mUpperBound = PM::pm_SetIndex(mUpperBound, i, PM::pm_GetIndex(point, i));
			}
			else if (PM::pm_GetIndex(point, i) < PM::pm_GetIndex(mLowerBound, i))
			{
				mLowerBound = PM::pm_SetIndex(mLowerBound, i, PM::pm_GetIndex(point, i));
			}
		}
	}

	void BoundingBox::combine(const BoundingBox& other)
	{
		if (!isValid())
		{
			*this = other;
			return;
		}

		put(other.upperBound());
		put(other.lowerBound());
	}

	Plane BoundingBox::getFace(FaceSide side) const
	{
		PR_GUARD_PROFILE();
		
		PM::vec3 diff = PM::pm_Subtract(mUpperBound, mLowerBound);

		switch (side)
		{
		default:
		case FS_Front:
			return Plane(mLowerBound, PM::pm_Set(PM::pm_GetX(diff), 0, 0), PM::pm_Set(0, PM::pm_GetY(diff), 0));
		case FS_Back:
			return Plane(PM::pm_Set(PM::pm_GetX(mUpperBound), PM::pm_GetY(mLowerBound), PM::pm_GetZ(mUpperBound), 1),
				PM::pm_Set(-PM::pm_GetX(diff), 0, 0),
				PM::pm_Set(0, PM::pm_GetY(diff), 0));
		case FS_Left:
			return Plane(PM::pm_Set(PM::pm_GetX(mLowerBound), PM::pm_GetY(mLowerBound), PM::pm_GetZ(mUpperBound), 1),
				PM::pm_Set(0, 0, -PM::pm_GetZ(diff)),
				PM::pm_Set(0, PM::pm_GetY(diff), 0));
		case FS_Right:
			return Plane(PM::pm_Set(PM::pm_GetX(mUpperBound), PM::pm_GetY(mLowerBound), PM::pm_GetZ(mLowerBound), 1),
				PM::pm_Set(0, 0, PM::pm_GetZ(diff)),
				PM::pm_Set(0, PM::pm_GetY(diff), 0));
		case FS_Top:
			return Plane(PM::pm_Set(PM::pm_GetX(mLowerBound), PM::pm_GetY(mUpperBound), PM::pm_GetZ(mLowerBound), 1),
				PM::pm_Set(PM::pm_GetX(diff), 0),
				PM::pm_Set(0, 0, PM::pm_GetZ(diff)));
		case FS_Bottom:
			return Plane(PM::pm_Set(PM::pm_GetX(mUpperBound), PM::pm_GetY(mLowerBound), PM::pm_GetZ(mLowerBound), 1),
				PM::pm_Set(-PM::pm_GetX(diff), 0),
				PM::pm_Set(0, 0, PM::pm_GetZ(diff)));
		}
	}
}