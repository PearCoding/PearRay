#pragma once

#include "ray/RayPackage.h"

namespace PR {
class SingleCollisionOutput;
class CollisionOutput;

/* Origin based sphere */
class PR_LIB Sphere {
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

	inline float volume() const { return (PR_PI * 4.0 / 3) * mRadius * mRadius * mRadius; }
	inline float surfaceArea() const { return PR_PI * 4 * mRadius * mRadius; }

	inline bool isValid() const { return mRadius > 0; }

	Vector3f normalPoint(float u, float v) const;
	Vector3f surfacePoint(float u, float v) const;

	Vector2f project(const Vector3f& p) const;
	Vector2fv project(const Vector3fv& p) const;

	inline bool contains(const Vector3f& point) const
	{
		return point.squaredNorm() <= mRadius * mRadius;
	}

	void intersects(const Ray& ray, SingleCollisionOutput& out) const;
	void intersects(const RayPackage& in, CollisionOutput& out) const;

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

private:
	float mRadius;
};
} // namespace PR
