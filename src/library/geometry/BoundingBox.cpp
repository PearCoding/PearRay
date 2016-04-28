#include "BoundingBox.h"
#include "Plane.h"

#include "ray/Ray.h"

namespace PR
{
	BoundingBox::BoundingBox() :
		mUpperBound(PM::pm_Set(0, 0, 0, 1)), mLowerBound(PM::pm_Set(0, 0, 0, 1))
	{
	}

	BoundingBox::BoundingBox(const PM::vec3& upperbound, const PM::vec3& lowerbound) :
		mUpperBound(upperbound), mLowerBound(lowerbound)
	{
	}

	BoundingBox::BoundingBox(float width, float height, float depth) :
		mUpperBound(PM::pm_Set(width/2, height/2, depth/2, 1)),
		mLowerBound(PM::pm_Set(-width/2, -height/2, -depth/2, 1))
	{
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

	PM::vec3 BoundingBox::upperBound() const
	{
		return mUpperBound;
	}

	void BoundingBox::setUpperBound(const PM::vec3& bound)
	{
		PM::pm_Copy(mUpperBound, bound);
	}

	PM::vec3 BoundingBox::lowerBound() const
	{
		return mLowerBound;
	}

	void BoundingBox::setLowerBound(const PM::vec3& bound)
	{
		PM::pm_Copy(mLowerBound, bound);
	}

	PM::vec3 BoundingBox::center() const
	{
		return PM::pm_Add(mLowerBound, PM::pm_Scale(PM::pm_Subtract(mUpperBound, mLowerBound), 0.5f));
	}

	float BoundingBox::width() const
	{
		return std::abs(PM::pm_GetX(mUpperBound) - PM::pm_GetX(mLowerBound));
	}

	float BoundingBox::height() const
	{
		return std::abs(PM::pm_GetY(mUpperBound) - PM::pm_GetY(mLowerBound));
	}

	float BoundingBox::depth() const
	{
		return std::abs(PM::pm_GetZ(mUpperBound) - PM::pm_GetZ(mLowerBound));
	}

	float BoundingBox::volume() const
	{
		return width()*height()*depth();
	}

	bool BoundingBox::isValid() const
	{
		return volume() != 0;
	}

	bool BoundingBox::contains(const PM::vec3& point) const
	{
		return PM::pm_IsLessOrEqual(mUpperBound, point) &&
			PM::pm_IsGreaterOrEqual(mLowerBound, point);
	}

	bool BoundingBox::intersects(const Ray& ray) const
	{
		PM::vec3 tmp = PM::pm_Set(0,0,0,1);
		return intersects(ray, tmp);
	}

	bool BoundingBox::intersects(const Ray& ray, PM::vec3& collisionPoint) const
	{
		float tmin = -std::numeric_limits<float>::max();
		float tmax = std::numeric_limits<float>::max();

		float hx = width() / 2;
		float hy = height() / 2;
		float hz = depth() / 2;

		PM::vec3 p = PM::pm_Subtract(center(), ray.startPosition());

		// X
		if (std::abs(PM::pm_GetX(ray.direction())) > std::numeric_limits<float>::epsilon())
		{
			float t1 = (PM::pm_GetX(p) + hx) / PM::pm_GetX(ray.direction());
			float t2 = (PM::pm_GetX(p) - hx) / PM::pm_GetX(ray.direction());

			if (t1 > t2)
			{
				float tmp = t1;
				t1 = t2; t2 = tmp;
			}

			if (t1 > tmin)
			{
				tmin = t1;
			}

			if (t2 < tmax)
			{
				tmax = t2;
			}

			if (tmin > tmax || tmax < 0)
			{
				return false;
			}
		}
		else if (-PM::pm_GetX(p) - hx > 0 ||
			-PM::pm_GetX(p) + hx < 0)
		{
			return false;
		}

		// Y
		if (std::abs(PM::pm_GetY(ray.direction())) > std::numeric_limits<float>::epsilon())
		{
			float t1 = (PM::pm_GetY(p) + hy) / PM::pm_GetY(ray.direction());
			float t2 = (PM::pm_GetY(p) - hy) / PM::pm_GetY(ray.direction());

			if (t1 > t2)
			{
				float tmp = t1;
				t1 = t2; t2 = tmp;
			}

			if (t1 > tmin)
			{
				tmin = t1;
			}

			if (t2 < tmax)
			{
				tmax = t2;
			}

			if (tmin > tmax || tmax < 0)
			{
				return false;
			}
		}
		else if (-PM::pm_GetY(p) - hy > 0 ||
			-PM::pm_GetY(p) + hy < 0)
		{
			return false;
		}

		// Z
		if (std::abs(PM::pm_GetZ(ray.direction())) > std::numeric_limits<float>::epsilon())
		{
			float t1 = (PM::pm_GetZ(p) + hz) / PM::pm_GetZ(ray.direction());
			float t2 = (PM::pm_GetZ(p) - hz) / PM::pm_GetZ(ray.direction());

			if (t1 > t2)
			{
				float tmp = t1;
				t1 = t2; t2 = tmp;
			}

			if (t1 > tmin)
			{
				tmin = t1;
			}

			if (t2 < tmax)
			{
				tmax = t2;
			}

			if (tmin > tmax || tmax < 0)
			{
				return false;
			}
		}
		else if (-PM::pm_GetZ(p) - hz > 0 ||
			-PM::pm_GetZ(p) + hz < 0)
		{
			return false;
		}

		collisionPoint = PM::pm_Add(ray.startPosition(), PM::pm_Scale(ray.direction(),
			tmin <= 0 ? tmax : tmin));

		return true;
	}

	void BoundingBox::put(const PM::vec3& point)
	{
		/*if (!isValid())
		{
			PM::pm_Copy(mUpperBound, point);
			PM::pm_Copy(mLowerBound, point);
			return;
		}*/

		// UpperBound
		if (PM::pm_GetX(point) > PM::pm_GetX(mUpperBound))
		{
			mUpperBound = PM::pm_SetX(mUpperBound, PM::pm_GetX(point));
		}

		if (PM::pm_GetY(point) > PM::pm_GetY(mUpperBound))
		{
			mUpperBound = PM::pm_SetY(mUpperBound, PM::pm_GetY(point));
		}

		if (PM::pm_GetZ(point) > PM::pm_GetZ(mUpperBound))
		{
			mUpperBound = PM::pm_SetZ(mUpperBound, PM::pm_GetZ(point));
		}

		// LowerBound
		if (PM::pm_GetX(point) < PM::pm_GetX(mLowerBound))
		{
			mLowerBound = PM::pm_SetX(mLowerBound, PM::pm_GetX(point));
		}

		if (PM::pm_GetY(point) < PM::pm_GetY(mLowerBound))
		{
			mLowerBound = PM::pm_SetY(mLowerBound, PM::pm_GetY(point));
		}

		if (PM::pm_GetZ(point) < PM::pm_GetZ(mLowerBound))
		{
			mLowerBound = PM::pm_SetZ(mLowerBound, PM::pm_GetZ(point));
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

	void BoundingBox::shift(const PM::vec3& point)
	{
		mLowerBound = PM::pm_Add(mLowerBound, point);
		mUpperBound = PM::pm_Add(mUpperBound, point);
	}

	BoundingBox BoundingBox::putted(const PM::vec3& point) const
	{
		BoundingBox tmp = *this;
		tmp.put(point);
		return tmp;
	}

	BoundingBox BoundingBox::combined(const BoundingBox& other) const
	{
		BoundingBox tmp = *this;
		tmp.combine(other);
		return tmp;
	}

	BoundingBox BoundingBox::shifted(const PM::vec3& point) const
	{
		BoundingBox tmp = *this;
		tmp.shift(point);
		return tmp;
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