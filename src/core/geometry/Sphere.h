#pragma once

#include "ray/Ray.h"

namespace PR {
struct HitPoint;
struct CollisionOutput;

/* Origin based sphere */
class PR_LIB_CORE Sphere {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Sphere();
	Sphere(float radius);

	Sphere(const Sphere& other) = default;
	Sphere& operator=(const Sphere& other) = default;

	inline float radius() const { return mRadius; }
	inline void setRadius(float f)
	{
		PR_ASSERT(f > 0, "radius has to be greater than 0");
		mRadius = f;
	}

	inline float volume() const { return (PR_PI * 4.0f / 3) * mRadius * mRadius * mRadius; }
	inline float surfaceArea() const { return PR_PI * 4 * mRadius * mRadius; }

	inline bool isValid() const { return mRadius > 0; }

	Vector3f normalPoint(float u, float v) const;
	Vector3f surfacePoint(float u, float v) const;

	Vector2f project(const Vector3f& p) const;

	inline bool contains(const Vector3f& point) const
	{
		return point.squaredNorm() <= mRadius * mRadius;
	}

	void intersects(const Ray& ray, HitPoint& out) const;

	void combine(const Vector3f& point);
	void combine(const Sphere& other);

	inline Sphere combined(const Vector3f& point) const
	{
		Sphere tmp = *this;
		tmp.combine(point);
		return tmp;
	}

	inline Sphere combined(const Sphere& other) const
	{
		Sphere tmp = *this;
		tmp.combine(other);
		return tmp;
	}

    static void triangulate(const Vector3f& center, float radius, uint32 stacks, uint32 slices, std::vector<float>& vertices);
    static void triangulateIndices(uint32 stacks, uint32 slices, std::vector<uint32>& indices, uint32 off = 0);

private:
	float mRadius;
};
} // namespace PR
