#pragma once

#include "geometry/BoundingBox.h"

namespace PR {

// Disk centered on the origin with normal (0,0,1)
class PR_LIB_CORE Disk {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Disk(float radius = 1.0f);

	inline float radius() const { return mRadius; }
	void setRadius(float radius);

	inline Vector3f normal() const { return Vector3f(0, 0, 1); }

	inline float surfaceArea() const { return PR_PI * mRadius * mRadius; }
	inline Vector3f surfacePoint(float u, float v) const
	{
		return Vector3f(
			mRadius * v * std::cos(2 * PR_PI * u),
			mRadius * v * std::sin(2 * PR_PI * u),
			0);
	}

	inline bool isValid() const
	{
		return std::abs(mRadius) > PR_EPSILON;
	}

	bool contains(const Vector3f& point) const;

	void intersects(const Ray& ray, HitPoint& out) const;

	Vector2f project(const Vector3f& point) const;

	inline BoundingBox toBoundingBox() const
	{
		constexpr float INF_EXP = 0.0001f;
		BoundingBox box;
		box.combine(Vector3f(mRadius, 0, INF_EXP));
		box.combine(Vector3f(-mRadius, 0, -INF_EXP));
		box.combine(Vector3f(0, mRadius, INF_EXP));
		box.combine(Vector3f(0, -mRadius, -INF_EXP));
		return box;
	}

	static void triangulate(const Vector3f& center, float radius, uint32 sectionCount, std::vector<float>& indices);
	static void triangulateIndices(uint32 centerID, uint32 sectionCount, std::vector<uint32>& indices, uint32 off = 1);

private:
	float mRadius;
};
} // namespace PR
