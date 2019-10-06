#pragma once

#include "ray/RayPackage.h"

namespace PR {
class SingleCollisionOutput;
class CollisionOutput;
class PR_LIB Sphere {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Sphere();
	Sphere(const Vector3f& pos, float radius);

	Sphere(const Sphere& other);
	Sphere& operator=(const Sphere& other);

	inline Vector3f position() const { return mPosition; }
	inline void setPosition(const Vector3f& pos) { mPosition = pos; }

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
		return (mPosition - point).squaredNorm() <= mRadius * mRadius;
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
	Vector3f mPosition;
	float mRadius;
};
} // namespace PR
