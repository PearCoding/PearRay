#pragma once

#include "PR_Config.h"
#include "PearMath.h"

namespace PR
{
	class Ray;
	class PR_LIB Sphere
	{
	public:
		Sphere();
		Sphere(PM::vec3 pos, float radius);

		Sphere(const Sphere& other);
		Sphere& operator = (const Sphere& other);

		inline PM::vec3 position() const { return mPosition; }
		inline void setPosition(const PM::vec3& pos) { mPosition = pos; }

		inline float radius() const { return mRadius; }
		inline void setRadius(float f)
		{
			PR_ASSERT(f > 0, "radius has to be greater than 0");
			mRadius = f;
		}

		inline float volume() const { return (PM_4_PI_F/3)*mRadius*mRadius*mRadius; }
		inline float surfaceArea() const { return PM_4_PI_F*mRadius*mRadius; }

		inline bool isValid() const { return mRadius > 0; }

		inline bool contains(const PM::vec3& point) const
		{
			return PM::pm_MagnitudeSqr(PM::pm_Subtract(mPosition, point)) <= mRadius*mRadius;
		}

		bool intersects(const Ray& ray, PM::vec3& collisionPoint, float& t) const;

		void put(const PM::vec3& point);
		void combine(const Sphere& other);

		inline Sphere putted(const PM::vec3& point) const
		{
			Sphere tmp = *this;
			tmp.put(point);
			return tmp;
		}

		inline Sphere combined(const Sphere& other) const
		{
			Sphere tmp = *this;
			tmp.combine(other);
			return tmp;
		}

	private:
		PM::vec3 mPosition;
		float mRadius;
	};
}
