#pragma once

#include "PR_Config.h"

#include <vector>
#include <array>

namespace PR {
// Bezier Spline
template <typename V, int Deg>
class Curve {
public:
	constexpr static int Degree = Deg;
	typedef std::array<V, Deg + 1> PointList;

	Curve() = default;
	inline Curve(const std::array<V, Deg + 1>& points) : mPoints(points) {}

	Curve(const Curve<V, Deg>& other) = default;
	Curve(Curve<V, Deg>&& other)	  = default;

	Curve& operator=(const Curve<V, Deg>& other) = default;
	Curve& operator=(Curve<V, Deg>&& other) = default;

	inline const V& operator[](size_t i) const { return mPoints[i]; }
	inline V& operator[](size_t i) { return mPoints[i]; }

	inline PointList& points() { return mPoints; }
	inline const PointList& points() const { return mPoints; }

	inline V eval(float t) const
	{
		return de_casteljau(t, 0, Degree);
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

template <int Deg>
using Curve1 = Curve<float, Deg>;
template <int Deg>
using Curve2 = Curve<Vector2f, Deg>;
template <int Deg>
using Curve3 = Curve<Vector3f, Deg>;
template <int Deg>
using Curve4 = Curve<Vector4f, Deg>;

} // namespace PR
