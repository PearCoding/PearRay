#pragma once

#include "geometry/BoundingBox.h"

namespace PR {
class Ray;
class PR_LIB Plane {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Plane();
	Plane(const Eigen::Vector3f& pos, const Eigen::Vector3f& xAxis, const Eigen::Vector3f& yAxis);
	Plane(float width, float height);

	inline Eigen::Vector3f position() const { return mPosition; }
	inline void setPosition(const Eigen::Vector3f& pos) { mPosition = pos; }

	inline Eigen::Vector3f xAxis() const { return mXAxis; }
	void setXAxis(const Eigen::Vector3f& xAxis);

	inline Eigen::Vector3f yAxis() const { return mYAxis; }
	void setYAxis(const Eigen::Vector3f& yAxis);

	void setAxis(const Eigen::Vector3f& xAxis, const Eigen::Vector3f& yAxis);

	inline Eigen::Vector3f normal() const { return mNormal_Cache; }
	inline Eigen::Vector3f center() const
	{
		return mPosition + mXAxis * 0.5f + mYAxis * 0.5f;
	}

	inline float surfaceArea() const { return mSurfaceArea_Cache; }

	inline bool isValid() const
	{
		return mXAxis.squaredNorm() * mYAxis.squaredNorm() > 0;
	}

	bool contains(const Eigen::Vector3f& point) const;

	struct Intersection {
		bool Successful;
		Eigen::Vector3f Position;
		float T;
		Eigen::Vector2f UV;
	};
	Intersection intersects(const Ray& ray) const;

	Eigen::Vector2f project(const Eigen::Vector3f& point) const;

	inline BoundingBox toBoundingBox() const
	{
		BoundingBox box = toLocalBoundingBox();
		box.shift(mPosition);
		return box;
	}

	BoundingBox toLocalBoundingBox() const;

private:
	void recache();

	Eigen::Vector3f mPosition;
	Eigen::Vector3f mXAxis;
	Eigen::Vector3f mYAxis;

	// Cache
	Eigen::Vector3f mNormal_Cache;
	float mSurfaceArea_Cache;
	float mInvXLenSqr_Cache;
	float mInvYLenSqr_Cache;
};
}
