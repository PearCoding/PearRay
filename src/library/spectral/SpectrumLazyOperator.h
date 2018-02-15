#pragma once

#include "PR_Config.h"
#include <type_traits>

namespace PR {

class Spectrum;
class SpectrumDescriptor;
namespace Lazy {
class SLO_Base {
};

template <typename T1>
class SLO_UnaryBase : public SLO_Base {
public:
	inline explicit SLO_UnaryBase(const T1& arg1)
		: SLO_Base()
		, mArg1(arg1)
	{
	}

	inline const std::shared_ptr<SpectrumDescriptor>& descriptor() const { return mArg1.descriptor(); }

	inline uint32 samples() const { return mArg1.samples(); }
	inline uint32 spectralStart() const { return mArg1.spectralStart(); }
	inline uint32 spectralEnd() const { return mArg1.spectralEnd(); }

protected:
	T1 mArg1;
};

template <typename T1, typename T2>
class SLO_Add : public SLO_UnaryBase<T1> {
public:
	inline SLO_Add(const T1& arg1, const T2& arg2)
		: SLO_UnaryBase<T1>(arg1)
		, mArg2(arg2)
	{
	}
	inline float operator()(uint32 i) const { return SLO_UnaryBase<T1>::mArg1(i) + mArg2(i); }

private:
	T2 mArg2;
};

template <typename T1, typename T2>
class SLO_Sub : public SLO_UnaryBase<T1> {
public:
	inline SLO_Sub(const T1& arg1, const T2& arg2)
		: SLO_UnaryBase<T1>(arg1)
		, mArg2(arg2)
	{
	}
	inline float operator()(uint32 i) const { return SLO_UnaryBase<T1>::mArg1(i) - mArg2(i); }

private:
	T2 mArg2;
};

template <typename T1, typename T2>
class SLO_Mul : public SLO_UnaryBase<T1> {
public:
	inline SLO_Mul(const T1& arg1, const T2& arg2)
		: SLO_UnaryBase<T1>(arg1)
		, mArg2(arg2)
	{
	}
	inline float operator()(uint32 i) const { return SLO_UnaryBase<T1>::mArg1(i) * mArg2(i); }

private:
	T2 mArg2;
};

template <typename T1, typename T2>
class SLO_Div : public SLO_UnaryBase<T1> {
public:
	inline SLO_Div(const T1& arg1, const T2& arg2)
		: SLO_UnaryBase<T1>(arg1)
		, mArg2(arg2)
	{
	}
	inline float operator()(uint32 i) const { return SLO_UnaryBase<T1>::mArg1(i) / mArg2(i); }

private:
	T2 mArg2;
};

// Spec + Float Operators
template <typename T1, typename T2>
class SLO_Scale : public SLO_UnaryBase<T1> {
public:
	inline SLO_Scale(const T1& arg1, T2 arg2)
		: SLO_UnaryBase<T1>(arg1)
		, mArg2(arg2)
	{
	}

	inline float operator()(uint32 i) const { return SLO_UnaryBase<T1>::mArg1(i) * mArg2; }

private:
	T2 mArg2;
};

template <typename T1, typename T2>
class SLO_InvScale : public SLO_UnaryBase<T1> {
public:
	inline SLO_InvScale(const T1& arg1, T2 arg2)
		: SLO_UnaryBase<T1>(arg1)
		, mArg2(arg2)
	{
	}
	inline float operator()(uint32 i) const { return mArg2 / SLO_UnaryBase<T1>::mArg1(i); }

private:
	T2 mArg2;
};

// Unary Operators
template <typename T1>
class SLO_Neg : public SLO_UnaryBase<T1> {
public:
	inline SLO_Neg(const T1& arg1)
		: SLO_UnaryBase<T1>(arg1)
	{
	}
	inline float operator()(uint32 i) const { return -SLO_UnaryBase<T1>::mArg1(i); }
};

template <typename T>
struct is_slo : public std::integral_constant<bool, std::is_base_of<SLO_Base, T>::value> {
};

template <typename T>
struct is_slo_callable : public std::integral_constant<bool, is_slo<T>::value || std::is_base_of<Spectrum, T>::value> {
};

template <typename T1, typename T2, typename RetT>
using enable_if_slo_t = std::enable_if_t<PR::Lazy::is_slo_callable<T1>::value && PR::Lazy::is_slo_callable<T2>::value, RetT>;

template <typename T1, typename T2, typename RetT>
using enable_if_slo_arith_t = std::enable_if_t<PR::Lazy::is_slo_callable<T1>::value && std::is_arithmetic<T2>::value, RetT>;
} // namespace Lazy
} // namespace PR

template <typename T1, typename T2, typename RetT = PR::Lazy::enable_if_slo_t<T1, T2, PR::Lazy::SLO_Add<T1, T2>>>
inline RetT operator+(const T1& t1, const T2& t2)
{
	return RetT(t1, t2);
}

template <typename T1, typename T2, typename RetT = PR::Lazy::enable_if_slo_t<T1, T2, PR::Lazy::SLO_Sub<T1, T2>>>
inline RetT operator-(const T1& t1, const T2& t2)
{
	return RetT(t1, t2);
}

template <typename T1, typename T2, typename RetT = PR::Lazy::enable_if_slo_t<T1, T2, PR::Lazy::SLO_Mul<T1, T2>>>
inline RetT operator*(const T1& t1, const T2& t2)
{
	return RetT(t1, t2);
}

template <typename T1, typename T2, typename RetT = PR::Lazy::enable_if_slo_t<T1, T2, PR::Lazy::SLO_Div<T1, T2>>>
inline RetT operator/(const T1& t1, const T2& t2)
{
	return RetT(t1, t2);
}

template <typename T1, typename T2, typename RetT = PR::Lazy::enable_if_slo_arith_t<T1, T2, PR::Lazy::SLO_Scale<T1, T2>>>
inline RetT operator*(const T1& t1, T2 t2)
{
	return RetT(t1, t2);
}

// Both arguments switched
template <typename T1, typename T2, typename RetT = PR::Lazy::enable_if_slo_arith_t<T1, T2, PR::Lazy::SLO_Scale<T1, T2>>>
inline RetT operator*(T2 t2, const T1& t1)
{
	return RetT(t1, t2);
}

template <typename T1, typename T2, typename RetT = PR::Lazy::enable_if_slo_arith_t<T1, T2, PR::Lazy::SLO_Scale<T1, T2>>>
inline RetT operator/(const T1& t1, T2 t2)
{
	return RetT(t1, 1 / t2);
}

template <typename T1, typename T2, typename RetT = PR::Lazy::enable_if_slo_arith_t<T1, T2, PR::Lazy::SLO_InvScale<T1, T2>>>
inline RetT operator/(T2 t2, const T1& t1)
{
	return RetT(t1, t2);
}

template <typename T1, typename RetT = PR::Lazy::enable_if_slo_t<T1, T1, T1>>
inline RetT operator+(const T1& t1)
{
	return t1;
}

template <typename T1, typename RetT = PR::Lazy::enable_if_slo_t<T1, T1, PR::Lazy::SLO_Neg<T1>>>
inline RetT operator-(const T1& t1)
{
	return RetT(t1);
}

template <typename T1, typename T2>
inline PR::Lazy::enable_if_slo_t<T1, T2, bool> operator==(const T1& v1, const T2& v2)
{
	if (v1.samples() != v2.samples())
		return false;

	for (PR::uint32 i = 0; i < v1.samples(); ++i) {
		if (v1(i) != v2(i))
			return false;
	}
	return true;
}

template <typename T1, typename T2>
inline PR::Lazy::enable_if_slo_t<T1, T2, bool> operator!=(const T1& v1, const T2& v2)
{
	return !(v1 == v2);
}
