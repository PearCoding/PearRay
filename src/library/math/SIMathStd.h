#pragma once
#include "SIMath.h"

#include <cmath>

namespace SI {
#define __SI_DEFINE_TRANS_FUNC_1(name)                                        \
	template <class Base, int Coordinate>                                     \
	inline ScalarU<Base, Coordinate> name(const ScalarU<Base, Coordinate>& v) \
	{                                                                         \
		return std::name(v.Value);                                            \
	}                                                                         \
	template <class Base, int Coordinate>                                     \
	inline RadianU<Base, Coordinate> name(const RadianU<Base, Coordinate>& v) \
	{                                                                         \
		return std::name(v.Value);                                            \
	}

__SI_DEFINE_TRANS_FUNC_1(exp)
__SI_DEFINE_TRANS_FUNC_1(exp2)
__SI_DEFINE_TRANS_FUNC_1(expm1)
__SI_DEFINE_TRANS_FUNC_1(log)
__SI_DEFINE_TRANS_FUNC_1(log10)
__SI_DEFINE_TRANS_FUNC_1(log2)
__SI_DEFINE_TRANS_FUNC_1(log1p)
__SI_DEFINE_TRANS_FUNC_1(sin)
__SI_DEFINE_TRANS_FUNC_1(cos)
__SI_DEFINE_TRANS_FUNC_1(tan)
__SI_DEFINE_TRANS_FUNC_1(asin)
__SI_DEFINE_TRANS_FUNC_1(acos)
__SI_DEFINE_TRANS_FUNC_1(atan)
__SI_DEFINE_TRANS_FUNC_1(sinh)
__SI_DEFINE_TRANS_FUNC_1(cosh)
__SI_DEFINE_TRANS_FUNC_1(tanh)
__SI_DEFINE_TRANS_FUNC_1(asinh)
__SI_DEFINE_TRANS_FUNC_1(acosh)
__SI_DEFINE_TRANS_FUNC_1(atanh)

template <typename Unit, int Exp>
typename std::enable_if<traits::is_unit<Unit>::value, typename details::unit_pow<Unit, std::ratio<Exp>>::type>::type
pow(const Unit& v)
{
	return std::pow(v.Value, Exp);
}

template <typename Unit, int Exp>
typename std::enable_if<traits::is_unit<Unit>::value, typename details::unit_sqrt<Unit>::type>::type
sqrt(const Unit& v)
{
	return std::sqrt(v.Value);
}
}