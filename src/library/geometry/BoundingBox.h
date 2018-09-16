#pragma once

#include "Sphere.h"
#include "CollisionData.h"
#include <Eigen/Geometry>

namespace PR {
class Ray;
class Plane;

/**
 * A axis aligned bounding box (AABB)
 */
class PR_LIB BoundingBox {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	enum FaceSide {
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
	BoundingBox(BoundingBox&& other)	  = default;
	BoundingBox& operator=(const BoundingBox& other) = default;
	BoundingBox& operator=(BoundingBox&& other) = default;

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

	inline float width() const { return edge(0); }

	inline float height() const { return edge(1); }

	inline float depth() const { return edge(2); }

	inline float edge(int dim) const { return mUpperBound(dim) - mLowerBound(dim); }

	inline float longestEdge() const { return std::max(width(), std::max(height(), depth())); }

	inline float diameter() const { return (mUpperBound - mLowerBound).norm(); }

	inline Sphere outerSphere()
	{
		return Sphere(center(),
					  std::max(width(), std::max(height(), depth())) * 0.5f * 1.73205080757f);
	}

	inline Sphere innerSphere()
	{
		return Sphere(center(),
					  std::max(width(), std::max(height(), depth())) * 0.5f);
	}

	inline float volume() const { return width() * height() * depth(); }
	inline float surfaceArea() const
	{
		return 2 * (width() * height() + width() * depth() + height() * depth());
	}

	inline float faceArea(int dim)
	{
		switch (dim) {
		case 0:
			return edge(1) * edge(2);
		case 1:
			return edge(0) * edge(2);
		case 2:
			return edge(0) * edge(1);
		default:
			return -1;
		}
	}

	inline bool isValid() const { return (mLowerBound.array() <= mUpperBound.array()).all(); }

	inline bool isPlanar(float eps = PR_EPSILON) const
	{
		return width() <= eps || height() <= eps || depth() <= eps;
	}

	inline void clipBy(const BoundingBox& other)
	{
		mLowerBound = mLowerBound.array().max(other.mLowerBound.array()).min(other.mUpperBound.array()).matrix();
		mUpperBound = mUpperBound.array().max(other.mLowerBound.array()).min(other.mUpperBound.array()).matrix();
	}
	/* Make sure bounding box has a valid volume.
	 * @param maxDir If true, will inflate in max direction, otherwise max and min direction.
	 */
	void inflate(float eps = 0.0001f, bool maxDir = false);

	inline bool contains(const Eigen::Vector3f& point) const
	{
		return (mUpperBound.array() <= point.array()).all() && (mLowerBound.array() >= point.array()).all();
	}

	struct Intersection {
		bool Successful;
		Eigen::Vector3f Position;
		float T;
	};
	Intersection intersects(const Ray& ray) const;
	bool intersectsSimple(const Ray& ray) const;
	void intersectsV(const CollisionInput& in, CollisionOutput& out) const;

	struct IntersectionRange {
		bool Successful;
		float Entry;
		float Exit;
	};
	IntersectionRange intersectsRange(const Ray& ray) const;

	struct IntersectionRangeV {
		simdpp::mask_float32v Successful;
		simdpp::float32v Entry;
		simdpp::float32v Exit;
	};
	IntersectionRangeV intersectsRangeV(const CollisionInput& in) const;

	FaceSide getIntersectionSide(const Intersection& intersection) const;

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
} // namespace PR
