#include "BoundingBox.h"

namespace PR
{
	BoundingBox::BoundingBox(const PM::vec3& upperbound, const PM::vec3& lowerbound) :
		UpperBound(upperbound), LowerBound(lowerbound)
	{
	}

	float BoundingBox::width() const
	{
		return std::fabsf(PM::pm_GetX(UpperBound) - PM::pm_GetX(LowerBound));
	}

	float BoundingBox::height() const
	{
		return std::fabsf(PM::pm_GetY(UpperBound) - PM::pm_GetY(LowerBound));
	}

	float BoundingBox::depth() const
	{
		return std::fabsf(PM::pm_GetZ(UpperBound) - PM::pm_GetZ(LowerBound));
	}

	float BoundingBox::volume() const
	{
		return width()*height()*depth();
	}

	bool BoundingBox::contains(const PM::vec3& point) const
	{
		return PM::pm_IsLessOrEqual(UpperBound, point) &&
			PM::pm_IsGreaterOrEqual(LowerBound, point);
	}

	void BoundingBox::put(const PM::vec3& point)
	{
		// UpperBound
		if (PM::pm_GetX(point) > PM::pm_GetX(UpperBound))
		{
			PM::pm_SetX(UpperBound, PM::pm_GetX(point));
		}

		if (PM::pm_GetY(point) > PM::pm_GetY(UpperBound))
		{
			PM::pm_SetY(UpperBound, PM::pm_GetY(point));
		}

		if (PM::pm_GetY(point) > PM::pm_GetY(UpperBound))
		{
			PM::pm_SetY(UpperBound, PM::pm_GetY(point));
		}

		// LowerBound
		if (PM::pm_GetX(point) < PM::pm_GetX(LowerBound))
		{
			PM::pm_SetX(LowerBound, PM::pm_GetX(point));
		}

		if (PM::pm_GetY(point) < PM::pm_GetY(LowerBound))
		{
			PM::pm_SetY(LowerBound, PM::pm_GetY(point));
		}

		if (PM::pm_GetY(point) < PM::pm_GetY(LowerBound))
		{
			PM::pm_SetY(LowerBound, PM::pm_GetY(point));
		}
	}

	void BoundingBox::combine(const BoundingBox& other)
	{
		put(other.UpperBound);
		put(other.LowerBound);
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
}