#pragma once

#include "PR_Config.h"

#include <array>
#include <vector>

namespace PR {
// Bezier Spline
template <typename V, typename StoreV>
class Curve {
public:
	typedef std::vector<StoreV> PointList;

	Curve() = default;
	inline explicit Curve(const PointList& points)
		: mPoints(points)
	{
	}
	inline explicit Curve(std::initializer_list<V>& points)
		: mPoints()
	{
		mPoints.reserve(points.size());
		for (auto it = points.begin(); it != points.end(); ++it)
			mPoints.emplace_back(StoreV(*it));
	}

	Curve(const Curve<V, StoreV>& other) = default;
	Curve(Curve<V, StoreV>&& other)		 = default;

	Curve& operator=(const Curve<V, StoreV>& other) = default;
	Curve& operator=(Curve<V, StoreV>&& other) = default;

	inline size_t degree() const { return mPoints.empty() ? 0 : mPoints.size() - 1; }
	inline bool isValid() const { return degree() > 0; }

	inline PointList& points() { return mPoints; }
	inline const PointList& points() const { return mPoints; }
	inline V point(size_t i) const
	{
		return mPoints.at(i);
	}

	inline V eval(float t) const
	{
		if (mPoints.empty())
			return V();

		return deCasteljau(t, 0, degree());
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

		return deCasteljau(params.data(), 0, degree());
	}

	template <size_t N>
	inline V evalBlossom(const std::array<float, N>& params) const
	{
		PR_ASSERT(params.size() == degree(),
				  "Expected params to be of the same size as the degree of the curve");
		if (mPoints.empty())
			return V();

		return deCasteljau(params.data(), 0, degree());
	}

private:
	inline V deCasteljau(float t, int i, size_t k) const
	{
		if (k == 0) {
			return point(i);
		} else {
			return (1 - t) * deCasteljau(t, i, k - 1)
				   + t * deCasteljau(t, i + 1, k - 1);
		}
	}

	inline V deCasteljau(const float* params, int i, size_t k) const
	{
		if (k == 0) {
			return point(i);
		} else {
			const float t = params[k - 1];
			return (1 - t) * deCasteljau(params, i, k - 1)
				   + t * deCasteljau(params, i + 1, k - 1);
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

using Curve1 = Curve<float, float>;
using Curve2 = Curve<Vector2f, Eigen::Matrix<float, 2, 1, Eigen::DontAlign>>;
using Curve3 = Curve<Vector3f, Eigen::Matrix<float, 3, 1, Eigen::DontAlign>>;
using Curve4 = Curve<Vector4f, Eigen::Matrix<float, 4, 1, Eigen::DontAlign>>;

} // namespace PR
