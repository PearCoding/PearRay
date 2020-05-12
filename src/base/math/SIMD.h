#pragma once

#include "PR_Config.h"

#include <boost/align/aligned_allocator.hpp>
#include <vector>

/* SIMD utility functions (extensions to libsimdpp)*/
namespace PR {
template <typename T>
using simd_vector = std::vector<T, boost::alignment::aligned_allocator<T, PR_SIMD_ALIGNMENT_PARAM>>;

inline uint32 countNegativeValues(const vfloat& v)
{
	uint32 r = 0;
	simdpp::for_each(v, [&](float f) { if(std::signbit(f)) ++r; });
	return r;
}

inline uint32 countNegativeValues(const vint32& v)
{
	uint32 r = 0;
	simdpp::for_each(v, [&](int32 f) { if(f < 0) ++r; });
	return r;
}

inline bool any(const bfloat& a)
{
	return simdpp::reduce_or(simdpp::bit_cast<vuint32::Base>(a));
}

inline bool all(const bfloat& a)
{
	return simdpp::reduce_and(simdpp::bit_cast<vuint32::Base>(a));
}

inline bool none(const bfloat& a)
{
	return !any(a);
}

template <unsigned int K>
inline bool extract(const bfloat& a)
{
	return simdpp::extract<K>(simdpp::bit_cast<vuint32::Base>(a)) != 0;
}

template <typename T>
inline auto b_not(const T& v)
{
	return ~v;
}

inline bool b_not(bool v)
{
	return !v;
}

template <typename T1, typename T2>
inline auto b_and(const T1& v1, const T2& v2)
{
	return v1 & v2;
}

inline bool b_and(bool v1, bool v2)
{
	return v1 && v2;
}

template <typename T1, typename T2>
inline auto b_or(const T1& v1, const T2& v2)
{
	return v1 | v2;
}

inline bool b_or(bool v1, bool v2)
{
	return v1 || v2;
}

template <typename C, unsigned N>
inline void load_from_container_linear(simdpp::float32<N>& out,
									   const C& container,
									   size_t off = 0)
{
	PR_SIMD_ALIGN
	float data[N];

	for (size_t i = 0; i < N; ++i) {
		data[i] = container[off + i];
	}

	out = simdpp::load(data);
}

template <typename C, unsigned N>
inline void load_from_container_linear(simdpp::uint32<N>& out,
									   const C& container,
									   size_t off = 0)
{
	PR_SIMD_ALIGN
	uint32 data[N];

	for (size_t i = 0; i < N; ++i) {
		data[i] = container[off + i];
	}

	out = simdpp::load(data);
}

template <typename C, unsigned N>
inline std::enable_if_t<std::is_floating_point<typename C::value_type>::value, simdpp::float32<N>>
load_from_container(const simdpp::uint32<N>& indices,
					const C& container,
					size_t off = 0)
{
	PR_SIMD_ALIGN
	uint32 ind[N];
	PR_SIMD_ALIGN
	float data[N];

	simdpp::store(ind, indices);
	for (size_t i = 0; i < N; ++i) {
		data[i] = container[ind[i] + off];
	}

	return simdpp::load(data);
}

template <typename C, unsigned N>
inline std::enable_if_t<std::is_integral<typename C::value_type>::value, simdpp::uint32<N>>
load_from_container(const simdpp::uint32<N>& indices,
					const C& container,
					size_t off = 0)
{
	PR_SIMD_ALIGN
	uint32 ind[N];
	PR_SIMD_ALIGN
	uint32 data[N];

	simdpp::store(ind, indices);
	for (size_t i = 0; i < N; ++i) {
		data[i] = container[ind[i] + off];
	}

	return simdpp::load(data);
}

template <typename C, unsigned N = PR_SIMD_BANDWIDTH>
inline std::enable_if_t<std::is_floating_point<typename C::value_type>::value, simdpp::float32<N>>
load_from_container_with_indices(const std::vector<size_t>& indices,
								 size_t ioff,
								 const C& container)
{
	PR_SIMD_ALIGN
	float data[N];

	for (size_t i = 0; i < N && i + ioff < indices.size(); ++i) {
		data[i] = container[indices[i + ioff]];
	}

	return simdpp::load(data);
}

template <typename C, unsigned N = PR_SIMD_BANDWIDTH>
inline std::enable_if_t<!std::is_floating_point<typename C::value_type>::value, simdpp::uint32<N>>
load_from_container_with_indices(const std::vector<size_t>& indices,
								 size_t ioff,
								 const C& container)
{
	PR_SIMD_ALIGN
	uint32 data[N];

	for (size_t i = 0; i < N && i + ioff < indices.size(); ++i) {
		data[i] = container[indices[i + ioff]];
	}

	return simdpp::load(data);
}

template <typename C, unsigned N>
inline std::enable_if_t<std::is_floating_point<typename C::value_type>::value>
store_into_container(const simdpp::uint32<N>& indices,
					 C& container,
					 const simdpp::float32<N>& val,
					 size_t off = 0)
{
	PR_SIMD_ALIGN
	uint32 ind[N];
	PR_SIMD_ALIGN
	float data[N];

	simdpp::store(ind, indices);
	simdpp::store(data, val);
	for (size_t i = 0; i < N; ++i) {
		container[ind[i] + off] = data[i];
	}
}

template <typename C, unsigned N>
inline std::enable_if_t<std::is_integral<typename C::value_type>::value>
store_into_container(const simdpp::uint32<N>& indices,
					 C& container,
					 const simdpp::uint32<N>& val,
					 size_t off = 0)
{
	PR_SIMD_ALIGN
	uint32 ind[N];
	PR_SIMD_ALIGN
	uint32 data[N];

	simdpp::store(ind, indices);
	simdpp::store(data, val);
	for (size_t i = 0; i < N; ++i) {
		container[ind[i] + off] = data[i];
	}
}

// Template to Runtime Bounded Range Cast
// -> Smart optimization will deduce nested if clausels

// ---- extract
template <size_t K>
class runtime_extract_H {
public:
	template <class... ArgTypes>
	inline auto operator()(size_t i, ArgTypes&&... args)
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
	inline auto operator()(size_t i, ArgTypes&&... args)
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
template <size_t K>
class runtime_insert_H {
public:
	template <class... ArgTypes>
	inline auto operator()(size_t i, ArgTypes&&... args)
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
	inline auto operator()(size_t i, ArgTypes&&... args)
	{
		if (i == 0)
			return simdpp::insert<0>(std::forward<ArgTypes>(args)...);
		else
			throw std::runtime_error("Invalid runtime argument");
	}
};

inline vfloat insert(size_t i, const vfloat& v, float f)
{
	return runtime_insert_H<PR_SIMD_BANDWIDTH - 1>()(i, v, f);
}

inline vuint32 insert(size_t i, const vuint32& v, uint32 f)
{
	return runtime_insert_H<PR_SIMD_BANDWIDTH - 1>()(i, v, f);
}

// for each
template <unsigned int K, typename F, typename S>
class runtime_foreach_H {
public:
	template <class... ArgTypes>
	inline void operator()(const S& val, F&& f, ArgTypes&&... args)
	{
		std::forward<F>(f)(simdpp::extract<K>(val), std::forward<ArgTypes>(args)...);
		runtime_foreach_H<K - 1, F, S>()(val, std::forward<F>(f), std::forward<ArgTypes>(args)...);
	}
};

template <typename F, typename S>
class runtime_foreach_H<0, F, S> {
public:
	template <class... ArgTypes>
	inline void operator()(const S& val, F&& f, ArgTypes&&... args)
	{
		std::forward<F>(f)(simdpp::extract<0>(val), std::forward<ArgTypes>(args)...);
	}
};

template <typename F, typename S, class... ArgTypes>
inline auto foreach_v(const S& val, F&& f, ArgTypes&&... args)
{
	return runtime_foreach_H<PR_SIMD_BANDWIDTH - 1, F, S>()(
		val, std::forward<F>(f), std::forward<ArgTypes>(args)...);
}

// for each with index
template <unsigned int K, typename F, typename S>
class runtime_foreach_index_H {
public:
	template <class... ArgTypes>
	inline void operator()(const S& val, F&& f, ArgTypes&&... args)
	{
		std::forward<F>(f)(K, simdpp::extract<K>(val), std::forward<ArgTypes>(args)...);
		runtime_foreach_index_H<K - 1, F, S>()(val, std::forward<F>(f), std::forward<ArgTypes>(args)...);
	}
};

template <typename F, typename S>
class runtime_foreach_index_H<0, F, S> {
public:
	template <class... ArgTypes>
	inline void operator()(const S& val, F&& f, ArgTypes&&... args)
	{
		std::forward<F>(f)(0, simdpp::extract<0>(val), std::forward<ArgTypes>(args)...);
	}
};

template <typename F, typename S, class... ArgTypes>
inline auto foreach_i_v(const S& val, F&& f, ArgTypes&&... args)
{
	return runtime_foreach_index_H<PR_SIMD_BANDWIDTH - 1, F, S>()(
		val, std::forward<F>(f), std::forward<ArgTypes>(args)...);
}

// for each with assignment
template <unsigned int K, typename F, typename S, typename S2>
class runtime_foreach_assign_H {
public:
	template <class... ArgTypes>
	inline void operator()(S& tmp, const S2& val, F&& f, ArgTypes&&... args)
	{
		tmp = simdpp::insert<K>(tmp, std::forward<F>(f)(simdpp::extract<K>(val), std::forward<ArgTypes>(args)...));
		runtime_foreach_assign_H<K - 1, F, S, S2>()(tmp, val, std::forward<F>(f), std::forward<ArgTypes>(args)...);
	}
};

template <typename F, typename S, typename S2>
class runtime_foreach_assign_H<0, F, S, S2> {
public:
	template <class... ArgTypes>
	inline void operator()(S& tmp, const S2& val, F&& f, ArgTypes&&... args)
	{
		tmp = simdpp::insert<0>(tmp, std::forward<F>(f)(simdpp::extract<0>(val), std::forward<ArgTypes>(args)...));
	}
};

template <typename F, typename S, class... ArgTypes>
inline auto foreach_assign_v(const S& val, F&& f, ArgTypes&&... args)
{
	S tmp;
	runtime_foreach_assign_H<PR_SIMD_BANDWIDTH - 1, F, S, S>()(
		tmp, val, std::forward<F>(f), std::forward<ArgTypes>(args)...);
	return tmp;
}

// for each with assignment and index
template <unsigned int K, typename F, typename S, typename S2>
class runtime_foreach_assign_index_H {
public:
	template <class... ArgTypes>
	inline void operator()(S& tmp, const S2& val, F&& f, ArgTypes&&... args)
	{
		tmp = simdpp::insert<K>(tmp, std::forward<F>(f)(K, simdpp::extract<K>(val), std::forward<ArgTypes>(args)...));
		runtime_foreach_assign_index_H<K - 1, F, S, S2>()(tmp, val, std::forward<F>(f), std::forward<ArgTypes>(args)...);
	}
};

template <typename F, typename S, typename S2>
class runtime_foreach_assign_index_H<0, F, S, S2> {
public:
	template <class... ArgTypes>
	inline void operator()(S& tmp, const S2& val, F&& f, ArgTypes&&... args)
	{
		tmp = simdpp::insert<0>(tmp, std::forward<F>(f)(0, simdpp::extract<0>(val), std::forward<ArgTypes>(args)...));
	}
};

template <typename F, typename S, class... ArgTypes>
inline auto foreach_assign_i_v(const S& val, F&& f, ArgTypes&&... args)
{
	S tmp;
	runtime_foreach_assign_index_H<PR_SIMD_BANDWIDTH - 1, F, S, S>()(
		tmp, val, std::forward<F>(f), std::forward<ArgTypes>(args)...);
	return tmp;
}
} // namespace PR