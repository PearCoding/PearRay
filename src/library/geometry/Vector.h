#pragma once

#include "PR_Config.h"

namespace PR {
/* Specialised vector container to be used inside lists or other data structures.
 * For computation use the eigen3 library instead!
 */
template <typename T>
struct PR_LIB_INLINE Vector2 {
	typedef Eigen::Matrix<T, 2, 1> eigen_t;
	T x;
	T y;

	Vector2() = default;
	inline Vector2(T x_, T y_)
		: x(x_)
		, y(y_)
	{
	}

	inline T operator[](int i) const
	{
		return i == 0 ? x : y;
	}

	inline operator eigen_t() const
	{
		return eigen_t(x, y);
	}
};

template <typename T>
struct PR_LIB_INLINE Vector3 {
	typedef Eigen::Matrix<T, 3, 1> eigen_t;
	T x;
	T y;
	T z;

	Vector3() = default;
	inline Vector3(T x_, T y_, T z_)
		: x(x_)
		, y(y_)
		, z(z_)
	{
	}

	inline T operator[](int i) const
	{
		return i == 0 ? x : (i == 1 ? y : z);
	}

	inline operator eigen_t() const
	{
		return eigen_t(x, y, z);
	}
};

typedef Vector2<float> Vector2f;
typedef Vector2<int32> Vector2i;
typedef Vector2<uint32> Vector2u;
typedef Vector2<int64> Vector2i64;
typedef Vector2<uint64> Vector2u64;

typedef Vector3<float> Vector3f;
typedef Vector3<int32> Vector3i;
typedef Vector3<uint32> Vector3u;
typedef Vector3<int64> Vector3i64;
typedef Vector3<uint64> Vector3u64;
} // namespace PR
