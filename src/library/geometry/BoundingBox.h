#pragma once

#include "Sphere.h"
#include <Eigen/Geometry>

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
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		
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
		BoundingBox(const Eigen::Vector3f& upperbound, const Eigen::Vector3f& lowerbound);
		BoundingBox(float width, float height, float depth);

		BoundingBox(const BoundingBox& other) = default;
		BoundingBox(BoundingBox&& other) = default;
		BoundingBox& operator = (const BoundingBox& other) = default;
		BoundingBox& operator = (BoundingBox&& other) = default;

		inline const Eigen::Vector3f& upperBound() const { return mUpperBound; }
		inline Eigen::Vector3f& upperBound() { return mUpperBound; }
		inline void setUpperBound(const Eigen::Vector3f& bound) { mUpperBound = bound; }

		inline const Eigen::Vector3f& lowerBound() const { return mLowerBound; }
		inline Eigen::Vector3f& lowerBound() { return mLowerBound; }
		inline void setLowerBound(const Eigen::Vector3f& bound) { mLowerBound = bound; }

		inline Eigen::Vector3f center() const 
		{
			return mLowerBound + (mUpperBound - mLowerBound) * 0.5f;
		}

		inline float width() const { return std::abs((mUpperBound-mLowerBound)(0)); }

		inline float height() const { return std::abs((mUpperBound-mLowerBound)(1)); }

		inline float depth() const { return std::abs((mUpperBound-mLowerBound)(2)); }

		inline Sphere outerSphere()
		{
			return Sphere(center(),
				std::max(width(), std::max(height(), depth()))*0.5f*1.73205080757f);
		}

		inline Sphere innerSphere()
		{
			return Sphere(center(),
				std::max(width(), std::max(height(), depth()))*0.5f);
		}

		inline float volume() const { return width()*height()*depth(); }
		inline float surfaceArea() const
		{
			return 2 * (width()*height() + width()*depth() + height()*depth());
		}

		inline bool isValid() const { return surfaceArea() > PR_EPSILON; }

		inline bool isPlanar() const
		{
			return width() <= PR_EPSILON || height() <= PR_EPSILON || depth() <= PR_EPSILON;
		}

		/* Make sure bounding box has a valid volume.
		 * @param maxDir If true, will inflate in max direction, otherwise max and min direction.
		 */
		void inflate(float eps=0.0001f, bool maxDir = false);

		inline bool contains(const Eigen::Vector3f& point) const 
		{
			return (mUpperBound.array() <= point.array()).all() &&
				(mLowerBound.array() >= point.array()).all();
		}

		bool intersects(const Ray& ray, float& t) const;
		bool intersects(const Ray& ray, Eigen::Vector3f& collisionPoint, float& t) const;
		bool intersects(const Ray& ray, Eigen::Vector3f& collisionPoint, float& t, FaceSide& side) const;

		void combine(const Eigen::Vector3f& point);
		void combine(const BoundingBox& other);

		inline void shift(const Eigen::Vector3f& point)
		{
			mUpperBound += point;
			mLowerBound += point;
		}

		inline BoundingBox combined(const Eigen::Vector3f& point) const
		{
			BoundingBox tmp = *this;
			tmp.combine(point);
			return tmp;
		}

		inline BoundingBox combined(const BoundingBox& other) const
		{
			BoundingBox tmp = *this;
			tmp.combine(other);
			return tmp;
		}

		inline BoundingBox shifted(const Eigen::Vector3f& point) const
		{
			BoundingBox tmp = *this;
			tmp.shift(point);
			return tmp;
		}

		Plane getFace(FaceSide side) const;
	private:
		Eigen::Vector3f mUpperBound;
		Eigen::Vector3f mLowerBound;
	};
}
