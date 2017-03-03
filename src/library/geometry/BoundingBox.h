#pragma once

#include "Sphere.h"

namespace PR
{
	class Ray;
	class Plane;

	/**
	 * A axis aligned bounding box (AABB)
	 */
	class PR_LIB BoundingBox
	{
	public:
		enum FaceSide
		{
			FS_Left,
			FS_Right,
			FS_Top,
			FS_Bottom,
			FS_Front,
			FS_Back
		};

		BoundingBox();
		BoundingBox(const PM::vec3& upperbound, const PM::vec3& lowerbound);
		BoundingBox(float width, float height, float depth);

		BoundingBox(const BoundingBox& other) = default;
		BoundingBox(BoundingBox&& other) = default;
		BoundingBox& operator = (const BoundingBox& other) = default;
		BoundingBox& operator = (BoundingBox&& other) = default;

		inline PM::vec3 upperBound() const { return mUpperBound; }
		inline void setUpperBound(const PM::vec3& bound) { mUpperBound = bound; }

		inline PM::vec3 lowerBound() const { return mLowerBound; }
		inline void setLowerBound(const PM::vec3& bound) { mLowerBound = bound; }

		inline PM::vec3 center() const
		{
			return PM::pm_Add(mLowerBound, PM::pm_Scale(PM::pm_Subtract(mUpperBound, mLowerBound), 0.5f));
		}

		inline float width() const
		{
			return std::abs(PM::pm_GetX(mUpperBound) - PM::pm_GetX(mLowerBound));
		}

		inline float height() const
		{
			return std::abs(PM::pm_GetY(mUpperBound) - PM::pm_GetY(mLowerBound));
		}

		inline float depth() const
		{
			return std::abs(PM::pm_GetZ(mUpperBound) - PM::pm_GetZ(mLowerBound));
		}

		inline Sphere outerSphere()
		{
			return Sphere(center(),
				PM::pm_Max(width(), PM::pm_Max(height(), depth()))*0.5f*1.73205080757f);
		}

		inline Sphere innerSphere()
		{
			return Sphere(center(),
				PM::pm_Max(width(), PM::pm_Max(height(), depth()))*0.5f);
		}

		inline float volume() const { return width()*height()*depth(); }
		inline float surfaceArea() const
		{
			return 2 * (width()*height() + width()*depth() + height()*depth());
		}

		inline bool isValid() const { return surfaceArea() > PM_EPSILON; }

		inline bool isPlanar() const
		{
			return width() <= PM_EPSILON || height() <= PM_EPSILON || depth() <= PM_EPSILON;
		}

		inline bool contains(const PM::vec3& point) const
		{
			return PM::pm_IsLessOrEqual(mUpperBound, point) &&
				PM::pm_IsGreaterOrEqual(mLowerBound, point);
		}

		bool intersects(const Ray& ray, float& t) const;
		bool intersects(const Ray& ray, PM::vec3& collisionPoint, float& t) const;
		bool intersects(const Ray& ray, PM::vec3& collisionPoint, float& t, FaceSide& side) const;

		void put(const PM::vec3& point);
		void combine(const BoundingBox& other);

		inline void shift(const PM::vec3& point)
		{
			mLowerBound = PM::pm_Add(mLowerBound, point);
			mUpperBound = PM::pm_Add(mUpperBound, point);
		}

		inline BoundingBox putted(const PM::vec3& point) const
		{
			BoundingBox tmp = *this;
			tmp.put(point);
			return tmp;
		}

		inline BoundingBox combined(const BoundingBox& other) const
		{
			BoundingBox tmp = *this;
			tmp.combine(other);
			return tmp;
		}

		inline BoundingBox shifted(const PM::vec3& point) const
		{
			BoundingBox tmp = *this;
			tmp.shift(point);
			return tmp;
		}

		Plane getFace(FaceSide side) const;
	private:
		PM::vec3 mUpperBound;
		PM::vec3 mLowerBound;
	};
}
