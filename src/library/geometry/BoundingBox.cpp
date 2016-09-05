#include "BoundingBox.h"
#include "Plane.h"

#include "ray/Ray.h"

#include <utility>

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
		float tmin = -std::numeric_limits<float>::max();
		float tmax = std::numeric_limits<float>::max();

		PM::vec3 d = PM::pm_Scale(PM::pm_Subtract(mUpperBound, mLowerBound), 0.5f);
		PM::vec3 p = PM::pm_Subtract(center(), ray.startPosition());

		PM::vec3 t1 = PM::pm_Divide(PM::pm_Add(p, d), PM::pm_SetW(ray.direction(), 1));
		PM::vec3 t2 = PM::pm_Divide(PM::pm_Subtract(p, d), PM::pm_SetW(ray.direction(), 1));

		for (int i = 0; i < 3; ++i)
		{
			if (std::abs(PM::pm_GetIndex(ray.direction(), i)) > PM_EPSILON)
			{
				float ft1 = PM::pm_GetIndex(t1, i);
				float ft2 = PM::pm_GetIndex(t2, i);

				if (ft1 > ft2)
					std::swap(ft1, ft2);

				if (ft1 > tmin)
					tmin = ft1;

				if (ft2 < tmax)
					tmax = ft2;

				if (tmin > tmax || tmax < 0)
					return false;
			}
			else if (-PM::pm_GetIndex(p, i) - PM::pm_GetIndex(d, i) > 0 ||
				-PM::pm_GetIndex(p, i) + PM::pm_GetIndex(d, i) < 0)
			{
				return false;
			}
		}

		t = tmin <= 0 ? tmax : tmin;
		if (t > PM_EPSILON)
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
		if (!intersects(ray, collisionPoint, t))
			return false;

		PM::vec3 minDist = PM::pm_Subtract(collisionPoint, mLowerBound);
		PM::vec3 maxDist = PM::pm_Subtract(collisionPoint, mUpperBound);

		minDist = PM::pm_Set(std::abs(PM::pm_GetX(minDist)), std::abs(PM::pm_GetY(minDist)), std::abs(PM::pm_GetZ(minDist)));
		maxDist = PM::pm_Set(std::abs(PM::pm_GetX(maxDist)), std::abs(PM::pm_GetY(maxDist)), std::abs(PM::pm_GetZ(maxDist)));

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