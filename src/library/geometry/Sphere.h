#pragma once

#include "PR_Config.h"
#include <Eigen/Dense>

namespace PR
{
	class Ray;
	class PR_LIB Sphere
	{
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		
		Sphere();
		Sphere(const Eigen::Vector3f& pos, float radius);

		Sphere(const Sphere& other);
		Sphere& operator = (const Sphere& other);

		inline Eigen::Vector3f position() const { return mPosition; }
		inline void setPosition(const Eigen::Vector3f& pos) { mPosition = pos; }

		inline float radius() const { return mRadius; }
		inline void setRadius(float f)
		{
			PR_ASSERT(f > 0, "radius has to be greater than 0");
			mRadius = f;
		}

		inline float volume() const { return (PR_PI*4.0/3)*mRadius*mRadius*mRadius; }
		inline float surfaceArea() const { return PR_PI*4*mRadius*mRadius; }

		inline bool isValid() const { return mRadius > 0; }

		inline bool contains(const Eigen::Vector3f& point) const
		{
			return (mPosition - point).squaredNorm() <= mRadius*mRadius;
		}

		bool intersects(const Ray& ray, Eigen::Vector3f& collisionPoint, float& t) const;

		void put(const Eigen::Vector3f& point);
		void combine(const Sphere& other);

		inline Sphere putted(const Eigen::Vector3f& point) const
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
		Eigen::Vector3f mPosition;
		float mRadius;
	};
}
