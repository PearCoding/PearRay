#pragma once

#include "PR_Config.h"

#include <array>
#include <vector>

namespace PR {
// Bezier Spline
template <typename V, typename List>
class Curve {
public:
	typedef List PointList;

	Curve() = default;
	inline explicit Curve(const PointList& points)
		: mPoints(points)
	{
	}

	Curve(const Curve<V, List>& other) = default;
	Curve(Curve<V, List>&& other)	  = default;

	Curve& operator=(const Curve<V, List>& other) = default;
	Curve& operator=(Curve<V, List>&& other) = default;

	inline size_t degree() const { return mPoints.empty() ? 0 : mPoints.size() - 1; }
	inline bool isValid() const { return degree() > 0; }

	inline void setPoints(const PointList& p) { mPoints = p; }
	inline PointList& points() { return mPoints; }
	inline const PointList& points() const { return mPoints; }
	inline V point(size_t i) const { return mPoints.at(i); }
	inline V startPoint() const { return point(0); }
	inline V endPoint() const { return point(degree()); }

	inline V eval(float t) const
	{
		if (mPoints.empty())
			return V();

		return deCasteljau(0, degree(),
						   [t](size_t) {
							   return t;
						   });
	}

	inline V evalDerivative(float t) const
	{
		if (mPoints.empty())
			return V();

		size_t n = degree();
		V v		 = bernstein(t, 0, n - 1) * (point(1) - point(0));
		for (size_t i = 1; i < n; ++i) {
			v += bernstein(t, (int)i, n - 1) * (point(i + 1) - point(i));
		}

		return n * v;
	}

	inline V evalBlossom(const std::vector<float>& params) const
	{
		PR_ASSERT(params.size() == degree(),
				  "Expected params to be of the same size as the degree of the curve");
		if (mPoints.empty())
			return V();

		return deCasteljau(0, degree(),
						   [&](size_t k) {
							   return params[k - 1];
						   });
	}

	template <size_t N>
	inline V evalBlossom(const std::array<float, N>& params) const
	{
		PR_ASSERT(params.size() == degree(),
				  "Expected params to be of the same size as the degree of the curve");
		if (mPoints.empty())
			return V();

		return deCasteljau(0, degree(),
						   [&](size_t k) {
							   return params[k - 1];
						   });
	}

	/* Implements blossom operation within given range:
		[0] = evalBlossom({u0,u0,u0})
		[1] = evalBlossom({u0,u0,u1})
		[2] = evalBlossom({u0,u1,u1})
		[3] = evalBlossom({u1,u1,u1})
	*/
	inline Curve blossom(float u0, float u1) const
	{
		Curve tmp = *this;
		for (size_t i = 0; i < degree() + 1; ++i)
			tmp.mPoints[i] = deCasteljau(0, degree(),
									   [&](size_t k) {
										   return k >= (degree() + 1 - i) ? u1 : u0;
									   });

		return tmp;
	}

	inline void subdivide(Curve& firstHalf, Curve& secondHalf) const
	{
		firstHalf  = blossom(0, 0.5f);
		secondHalf = blossom(0.5f, 1);
	}

private:
	template <typename Func>
	inline V deCasteljau(int i, size_t k, Func tFunc) const
	{
		if (k == 0) {
			return point(i);
		} else {
			const float t = tFunc(k);
			return (1 - t) * deCasteljau(i, k - 1, tFunc)
				   + t * deCasteljau(i + 1, k - 1, tFunc);
		}
	}

	inline float bernstein(float t, int i, size_t k) const
	{
		if (i < 0 || static_cast<size_t>(i) > k)
			return 0;
		else if (k == 0)
			return 1;
		else
			return (1 - t) * bernstein(t, i, k - 1) + t * bernstein(t, i - 1, k - 1);
	}

	PointList mPoints;
};

using Curve1 = Curve<float, std::vector<float>>;
using Curve2 = Curve<Vector2f, std::vector<Eigen::Matrix<float, 2, 1, Eigen::DontAlign>>>;
using Curve3 = Curve<Vector3f, std::vector<Eigen::Matrix<float, 3, 1, Eigen::DontAlign>>>;
using Curve4 = Curve<Vector4f, std::vector<Eigen::Matrix<float, 4, 1, Eigen::DontAlign>>>;

// Fixed variants -> Faster
template <size_t N>
using FixedCurve1 = Curve<float, std::array<float, N>>;
template <size_t N>
using FixedCurve2 = Curve<Vector2f, std::array<Vector2f, N>>;
template <size_t N>
using FixedCurve3 = Curve<Vector3f, std::array<Vector3f, N>>;
template <size_t N>
using FixedCurve4 = Curve<Vector4f, std::array<Vector4f, N>>;

// Quadratic
using QuadraticCurve1 = FixedCurve1<3>;
using QuadraticCurve2 = FixedCurve2<3>;
using QuadraticCurve3 = FixedCurve3<3>;
using QuadraticCurve4 = FixedCurve4<3>;

// Cubic
using CubicCurve1 = FixedCurve1<4>;
using CubicCurve2 = FixedCurve2<4>;
using CubicCurve3 = FixedCurve3<4>;
using CubicCurve4 = FixedCurve4<4>;

} // namespace PR
