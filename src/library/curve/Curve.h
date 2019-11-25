#pragma once

#include "PR_Config.h"

#include <array>
#include <vector>

namespace PR {
// Bezier Spline
template <typename V>
class Curve {
public:
	typedef std::vector<V> PointList;

	Curve() = default;
	inline Curve(const std::vector<V>& points)
		: mPoints(points)
	{
	}

	Curve(const Curve<V>& other) = default;
	Curve(Curve<V>&& other)		 = default;

	Curve& operator=(const Curve<V>& other) = default;
	Curve& operator=(Curve<V>&& other) = default;

	inline const V& operator[](size_t i) const { return mPoints[i]; }
	inline V& operator[](size_t i) { return mPoints[i]; }

	inline size_t degree() const { return mPoints.empty() ? 0 : mPoints.size() - 1; }
	inline bool isValid() const { return degree() > 0; }

	inline PointList& points() { return mPoints; }
	inline const PointList& points() const { return mPoints; }

	inline V eval(float t) const
	{
		if (mPoints.empty())
			return V(0);

		return de_casteljau(t, 0, degree());
	}

private:
	inline V de_casteljau(float t, int i, size_t k) const
	{
		if (k == 0) {
			return mPoints[i];
		} else {
			return (1 - t) * de_casteljau(t, i, k - 1)
				   + t * de_casteljau(t, i + 1, k - 1);
		}
	}

	PointList mPoints;
};

using Curve1 = Curve<float>;
using Curve2 = Curve<Vector2f>;
using Curve3 = Curve<Vector3f>;
using Curve4 = Curve<Vector4f>;

} // namespace PR
