#pragma once

#include "geometry/BoundingBox.h"

namespace PR {
class PR_LIB_CORE Plane {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Plane();
	Plane(const Vector3f& pos, const Vector3f& xAxis, const Vector3f& yAxis);
	Plane(float width, float height);

	inline Vector3f position() const { return mPosition; }
	inline void setPosition(const Vector3f& pos) { mPosition = pos; }

	inline Vector3f xAxis() const { return mXAxis; }
	void setXAxis(const Vector3f& xAxis);

	inline Vector3f yAxis() const { return mYAxis; }
	void setYAxis(const Vector3f& yAxis);

	void setAxis(const Vector3f& xAxis, const Vector3f& yAxis);

	inline Vector3f normal() const { return mNormal_Cache; }
	inline Vector3f center() const
	{
		return surfacePoint(0.5f, 0.5f);
	}

	inline float surfaceArea() const { return mSurfaceArea_Cache; }
	inline Vector3f surfacePoint(float u, float v) const
	{
		return mPosition + mXAxis * u + mYAxis * v;
	}

	inline bool isValid() const
	{
		return mXAxis.squaredNorm() * mYAxis.squaredNorm() > 0;
	}

	bool contains(const Vector3f& point) const;

	void intersects(const Ray& ray, HitPoint& out) const;

	Vector2f project(const Vector3f& point) const;

	inline BoundingBox toBoundingBox() const
	{
		BoundingBox box = toLocalBoundingBox();
		box.shift(mPosition);
		return box;
	}

	BoundingBox toLocalBoundingBox() const;

private:
	void recache();

	Vector3f mPosition;
	Vector3f mXAxis;
	Vector3f mYAxis;

	// Cache
	Vector3f mNormal_Cache;
	float mSurfaceArea_Cache;
	float mInvXLenSqr_Cache;
	float mInvYLenSqr_Cache;
};
} // namespace PR
