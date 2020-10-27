#pragma once

#include "ray/Ray.h"

namespace PR {
class Plane;
struct HitPoint;

/**
 * A axis aligned bounding box (AABB)
 */
class PR_LIB_CORE BoundingBox {
public:
	enum FaceSide {
		FS_Left,
		FS_Right,
		FS_Top,
		FS_Bottom,
		FS_Front,
		FS_Back
	};

	inline BoundingBox();
	inline BoundingBox(const Vector3f& upperbound, const Vector3f& lowerbound);
	inline BoundingBox(float width, float height, float depth);

	BoundingBox(const BoundingBox& other) = default;
	BoundingBox(BoundingBox&& other)	  = default;
	BoundingBox& operator=(const BoundingBox& other) = default;
	BoundingBox& operator=(BoundingBox&& other) = default;

	inline Vector3f upperBound() const;
	inline Vector3f& upperBound();
	inline void setUpperBound(const Vector3f& bound);

	inline Vector3f lowerBound() const;
	inline Vector3f& lowerBound();
	inline void setLowerBound(const Vector3f& bound);

	inline Vector3f center() const;

	inline float width() const;
	inline float height() const;
	inline float depth() const;
	inline float edge(int dim) const;

	inline float longestEdge() const;
	inline float shortestEdge() const;

	inline float diameter() const;

	inline float volume() const;
	inline float surfaceArea() const;

	inline float faceArea(int dim) const;

	inline bool isValid() const;
	inline bool isPlanar(float eps = PR_EPSILON) const;

	inline bool contains(const Vector3f& point) const;

	void intersects(const Ray& in, HitPoint& out) const;

	struct IntersectionRange {
		bool Successful;
		float Entry;
		float Exit;
	};
	IntersectionRange intersectsRange(const Ray& ray) const;

	FaceSide getIntersectionSide(const Vector3f& intersection) const;

	inline void clipBy(const BoundingBox& other);

	/* Make sure bounding box has a valid volume.
	 * @param maxDir If true, will inflate in max direction, otherwise max and min direction.
	 */
	void inflate(float eps = 0.0001f, bool maxDir = false);

	/**
	 * @brief Expand boundingbox of the given amount in each dimension
	 *
	 * @param amount Scale factor
	 */
	inline void expand(float amount);
	inline void combine(const Vector3f& point);
	inline void combine(const BoundingBox& other);
	inline void shift(const Vector3f& point);

	inline BoundingBox expanded(float amount) const;
	inline BoundingBox combined(const Vector3f& point) const;
	inline BoundingBox combined(const BoundingBox& other) const;
	inline BoundingBox shifted(const Vector3f& point) const;

	inline void transform(const Eigen::Affine3f& t);
	inline BoundingBox transformed(const Eigen::Affine3f& t) const;

	Plane getFace(FaceSide side) const;
	Vector3f corner(int n) const;

	static void triangulateIndices(const std::array<uint32, 8>& ids, std::vector<uint32>& indices);

private:
	Vector3f mUpperBound;
	Vector3f mLowerBound;
};
} // namespace PR

#include "BoundingBox.inl"
