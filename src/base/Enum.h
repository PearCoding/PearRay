#pragma once

#include "PR_Config.h"

#include <type_traits>

namespace PR {
template <typename E>
using enable_enum_t = typename std::enable_if<std::is_enum<E>::value,
											  typename std::underlying_type<E>::type>::type;

template <typename E>
constexpr inline enable_enum_t<E> underlying_value(E e) noexcept
{
	return static_cast<typename std::underlying_type<E>::type>(e);
}

template <typename E, typename T>
constexpr inline typename std::enable_if<std::is_enum<E>::value && std::is_integral<T>::value, E>::type
to_enum(T value) noexcept
{
	return static_cast<E>(value);
}

template <typename E>
struct PR_LIB_BASE FlagContainer {
	using type = typename std::underlying_type<E>::type;
	type value = 0;

	inline constexpr FlagContainer()		   = default;
	inline FlagContainer(const FlagContainer&) = default;
	inline FlagContainer& operator=(const FlagContainer&) = default;
	inline FlagContainer(FlagContainer&&)				  = default;
	inline FlagContainer& operator=(FlagContainer&&) = default;
	inline constexpr FlagContainer(const E& v)
		: value(underlying_value(v))
	{
	}
	template <typename T>
	inline constexpr FlagContainer(const T& v, typename std::enable_if<std::is_integral<T>::value>* = 0)
		: value(v)
	{
	}
	inline operator type() const { return value; }
	inline FlagContainer& operator|=(const FlagContainer& other)
	{
		value |= other.value;
		return *this;
	}
	inline FlagContainer& operator&=(const FlagContainer& other)
	{
		value &= other.value;
		return *this;
	}
};

template <typename E>
inline FlagContainer<E> operator&(const FlagContainer<E>& a, const E& b) { return a.value & PR::underlying_value(b); }
template <typename E>
inline FlagContainer<E> operator&(const E& a, const FlagContainer<E>& b) { return PR::underlying_value(a) & b.value; }
template <typename E>
inline FlagContainer<E> operator|(const FlagContainer<E>& a, const E& b) { return a.value | PR::underlying_value(b); }
template <typename E>
inline FlagContainer<E> operator|(const E& a, const FlagContainer<E>& b) { return PR::underlying_value(a) | b.value; }

} // namespace PR

#define PR_MAKE_FLAGS(single_type, cont_type)                  \
	using cont_type = typename PR::FlagContainer<single_type>; \
	inline cont_type operator|(const single_type& a, const single_type& b) { return cont_type(a) | b; }
