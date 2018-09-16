#pragma once

#include "CTP.h"

#include <boost/align/aligned_allocator.hpp>
#include <vector>

/* SIMD utility functions (extensions to libsimdpp)*/
namespace PR {
template <typename T>
using simd_vector = std::vector<T, boost::alignment::aligned_allocator<T, PR_SIMD_ALIGNMENT_PARAM>>;

/* Regardless of optimal size, we have to enforce same components for all vectorized types*/
constexpr size_t PR_SIMD_BANDWIDTH = SIMDPP_FAST_FLOAT32_SIZE;
using vfloat					   = simdpp::float32<PR_SIMD_BANDWIDTH>;
using vint32					   = simdpp::int32<PR_SIMD_BANDWIDTH>;
using vuint32					   = simdpp::uint32<PR_SIMD_BANDWIDTH>;
using bfloat					   = simdpp::mask_float32<PR_SIMD_BANDWIDTH>;
using bint32					   = simdpp::mask_int32<PR_SIMD_BANDWIDTH>;
using buint32					   = bint32;

inline bool any(const bfloat& a)
{
	return simdpp::reduce_or(simdpp::bit_cast<vuint32>(a));
}

inline bool all(const bfloat& a)
{
	return simdpp::reduce_and(simdpp::bit_cast<vuint32>(a));
}

template <unsigned int K>
inline bool extract(const bfloat& a)
{
	return simdpp::extract<K>(simdpp::bit_cast<vuint32>(a)) != 0;
}

template <typename T1, typename T2, typename T3>
inline void crossV(const T1& a1, const T1& a2, const T1& a3,
				   const T2& b1, const T2& b2, const T2& b3,
				   T3& c1, T3& c2, T3& c3)
{
	c1 = a2 * b3 - a3 * b2;
	c2 = a3 * b1 - a1 * b3;
	c3 = a1 * b2 - a2 * b1;
}

template <typename T1, typename T2>
inline T1 dotV(const T1& a1, const T1& a2, const T1& a3,
				 const T2& b1, const T2& b2, const T2& b3)
{
	return a1 * b1 + a2 * b2 + a3 * b3;
}

template <typename T>
inline void normalizeV(T& a1, T& a2, T& a3)
{
	using namespace simdpp;

	T h = 1 / sqrt(dotV(a1, a2, a3, a1, a2, a3));
	a1  = a1 * h;
	a2  = a2 * h;
	a3  = a3 * h;
}

template <typename T>
inline void transformV(const Eigen::Matrix3f& m,
					   const T& a1, const T& a2, const T& a3,
					   T& b1, T& b2, T& b3)
{
	b1 = m(0, 0) * a1 + m(0, 1) * a2 + m(0, 2) * a3;
	b2 = m(1, 0) * a1 + m(1, 1) * a2 + m(1, 2) * a3;
	b3 = m(2, 0) * a1 + m(2, 1) * a2 + m(2, 2) * a3;
}

template <typename T>
inline void transformV(const Eigen::Matrix4f& m,
					   const T& a1, const T& a2, const T& a3,
					   T& b1, T& b2, T& b3)
{
	b1 = m(0, 0) * a1 + m(0, 1) * a2 + m(0, 2) * a3 + m(0, 3);
	b2 = m(1, 0) * a1 + m(1, 1) * a2 + m(1, 2) * a3 + m(1, 3);
	b3 = m(2, 0) * a1 + m(2, 1) * a2 + m(2, 2) * a3 + m(2, 3);

	const T h = 1 / (m(3, 0) * a1 + m(3, 1) * a2 + m(3, 2) * a3 + m(3, 3));

	b1 = b1 * h;
	b2 = b2 * h;
	b3 = b3 * h;
}

template <typename C, unsigned N>
inline std::enable_if_t<std::is_floating_point<typename C::value_type>::value, simdpp::float32<N>>
load_from_container(const simdpp::uint32<N>& indices,
					const C& container,
					uint32 off = 0)
{
	SIMDPP_ALIGN(N * 4)
	uint32 ind[N];
	SIMDPP_ALIGN(N * 4)
	float data[N];

	simdpp::store(ind, indices);
	for (uint32 i = 0; i < N; ++i) {
		data[i] = container[ind[i] + off];
	}

	return simdpp::load(data);
}

template <typename C, unsigned N>
inline std::enable_if_t<std::is_integral<typename C::value_type>::value, simdpp::uint32<N>>
load_from_container(const simdpp::uint32<N>& indices,
					const C& container,
					uint32 off = 0)
{
	SIMDPP_ALIGN(N * 4)
	uint32 ind[N];
	SIMDPP_ALIGN(N * 4)
	uint32 data[N];

	simdpp::store(ind, indices);
	for (uint32 i = 0; i < N; ++i) {
		data[i] = container[ind[i] + off];
	}

	return simdpp::load(data);
}

template <typename C, unsigned N>
inline std::enable_if_t<std::is_floating_point<typename C::value_type>::value>
store_into_container(const simdpp::uint32<N>& indices,
					 C& container,
					 const simdpp::float32<N>& val,
					 uint32 off = 0)
{
	SIMDPP_ALIGN(N * 4)
	uint32 ind[N];
	SIMDPP_ALIGN(N * 4)
	float data[N];

	simdpp::store(ind, indices);
	simdpp::store(data, val);
	for (uint32 i = 0; i < N; ++i) {
		container[ind[i] + off] = data[i];
	}
}

template <typename C, unsigned N>
inline std::enable_if_t<std::is_integral<typename C::value_type>::value>
store_into_container(const simdpp::uint32<N>& indices,
					 C& container,
					 const simdpp::uint32<N>& val,
					 uint32 off = 0)
{
	SIMDPP_ALIGN(N * 4)
	uint32 ind[N];
	SIMDPP_ALIGN(N * 4)
	uint32 data[N];

	simdpp::store(ind, indices);
	simdpp::store(data, val);
	for (uint32 i = 0; i < N; ++i) {
		container[ind[i] + off] = data[i];
	}
}

// Template to Runtime Bounded Range Cast
// -> Smart optimization will deduce nested if clausels

// ---- extract
template <unsigned int K>
class runtime_extract_H {
public:
	template <class... ArgTypes>
	inline auto operator()(unsigned int i, ArgTypes&&... args)
	{
		if (i == K)
			return simdpp::extract<K>(std::forward<ArgTypes>(args)...);
		else
			return runtime_extract_H<K - 1>()(i, args...);
	}
};

template <>
class runtime_extract_H<0> {
public:
	template <class... ArgTypes>
	inline auto operator()(unsigned int i, ArgTypes&&... args)
	{
		if (i == 0)
			return simdpp::extract<0>(std::forward<ArgTypes>(args)...);
		else
			throw std::runtime_error("Invalid runtime argument");
	}
};

inline float extract(size_t i, const vfloat& v)
{
	return runtime_extract_H<PR_SIMD_BANDWIDTH - 1>()(i, v);
}

inline uint32 extract(size_t i, const vuint32& v)
{
	return runtime_extract_H<PR_SIMD_BANDWIDTH - 1>()(i, v);
}

// ------ insert
template <unsigned int K>
class runtime_insert_H {
public:
	template <class... ArgTypes>
	inline auto operator()(unsigned int i, ArgTypes&&... args)
	{
		if (i == K)
			return simdpp::insert<K>(std::forward<ArgTypes>(args)...);
		else
			return runtime_insert_H<K - 1>()(i, args...);
	}
};

template <>
class runtime_insert_H<0> {
public:
	template <class... ArgTypes>
	inline auto operator()(unsigned int i, ArgTypes&&... args)
	{
		if (i == 0)
			return simdpp::insert<0>(std::forward<ArgTypes>(args)...);
		else
			throw std::runtime_error("Invalid runtime argument");
	}
};

inline vfloat insert(size_t i, const vfloat& v, float& f)
{
	return runtime_insert_H<PR_SIMD_BANDWIDTH - 1>()(i, v, f);
}

inline vuint32 insert(size_t i, const vuint32& v, uint32& f)
{
	return runtime_insert_H<PR_SIMD_BANDWIDTH - 1>()(i, v, f);
}

// for each with index
template <unsigned int K, typename F>
class runtime_foreach_H {
public:
	template <class... ArgTypes>
	inline void operator()(F&& f, ArgTypes&&... args)
	{
		std::forward<F>(f)(K, std::forward<ArgTypes>(args)...);
		runtime_foreach_H<K - 1, F>()(std::forward<F>(f), std::forward<ArgTypes>(args)...);
	}
};

template <>
template <typename F>
class runtime_foreach_H<0, F> {
public:
	template <class... ArgTypes>
	inline void operator()(F&& f, ArgTypes&&... args)
	{
		std::forward<F>(f)(0, std::forward<ArgTypes>(args)...);
	}
};

template <typename F, class... ArgTypes>
inline auto for_each_v(F&& f, ArgTypes&&... args)
{
	return runtime_foreach_H<PR_SIMD_BANDWIDTH - 1, F>()(
		std::forward<F>(f), std::forward<ArgTypes>(args)...);
}
} // namespace PR