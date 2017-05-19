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

		inline const Eigen::Vector3f& upperBound() const { return mBox.max(); }
		inline Eigen::Vector3f& upperBound() { return mBox.max(); }
		inline void setUpperBound(const Eigen::Vector3f& bound) { mBox.max() = bound; }

		inline const Eigen::Vector3f& lowerBound() const { return mBox.min(); }
		inline Eigen::Vector3f& lowerBound() { return mBox.min(); }
		inline void setLowerBound(const Eigen::Vector3f& bound) { mBox.min() = bound; }

		inline Eigen::Vector3f center() const
		{
			return mBox.min() + (mBox.max() - mBox.min()) * 0.5f;
		}

		inline float width() const
		{
			return mBox.sizes()(0);
		}

		inline float height() const
		{
			return mBox.sizes()(1);
		}

		inline float depth() const
		{
			return mBox.sizes()(2);
		}

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

		inline float volume() const { return mBox.volume(); }
		inline float surfaceArea() const
		{
			return 2 * (width()*height() + width()*depth() + height()*depth());
		}

		inline bool isValid() const { return !mBox.isEmpty(); }

		inline bool isPlanar() const
		{
			return width() <= PR_EPSILON || height() <= PR_EPSILON || depth() <= PR_EPSILON;
		}

		inline bool contains(const Eigen::Vector3f& point) const { return mBox.contains(point); }

		bool intersects(const Ray& ray, float& t) const;
		bool intersects(const Ray& ray, Eigen::Vector3f& collisionPoint, float& t) const;
		bool intersects(const Ray& ray, Eigen::Vector3f& collisionPoint, float& t, FaceSide& side) const;

		inline void put(const Eigen::Vector3f& point) { mBox.extend(point); }
		inline void combine(const BoundingBox& other) { mBox.extend(other.mBox); }

		inline void shift(const Eigen::Vector3f& point)
		{
			mBox.translate(point);
		}

		inline BoundingBox putted(const Eigen::Vector3f& point) const
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

		inline BoundingBox shifted(const Eigen::Vector3f& point) const
		{
			BoundingBox tmp = *this;
			tmp.shift(point);
			return tmp;
		}

		Plane getFace(FaceSide side) const;
	private:
		Eigen::AlignedBox3f mBox;
	};
}
