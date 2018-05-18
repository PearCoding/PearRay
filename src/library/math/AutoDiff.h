#pragma once

#include "PR_Config.h"

namespace PR {

class AutoDiff_CoreBase {
};

template <typename T>
using is_ad = std::is_base_of<AutoDiff_CoreBase, T>;

template <typename T>
using is_ad_scalar = std::integral_constant<bool, is_ad<T>::value && T::_type::value == 0>;

template <typename T>
using is_ad_vector = std::integral_constant<bool, is_ad<T>::value && T::_type::value == 1>;

namespace Lazy {
class ADLO_Base {
};

template <typename T1>
class ADLO_UnaryBase : public ADLO_Base {
public:
	using _type1	= typename T1::_type;
	using value_t   = typename T1::value_t;
	using is_scalar = std::integral_constant<bool,
											 ADLO_UnaryBase<T1>::_type1::value == 0>;

	inline explicit ADLO_UnaryBase(const T1& arg1)
		: ADLO_Base()
		, mArg1(arg1)
	{
	}

protected:
	T1 mArg1;
};

template <typename T1, typename T2>
class ADLO_BinaryBase : public ADLO_UnaryBase<T1> {
public:
	using _type2 = typename T2::_type;
	using typename ADLO_UnaryBase<T1>::value_t;
	using is_scalar = std::integral_constant<bool,
											 ADLO_UnaryBase<T1>::_type1::value == 0 && _type2::value == 0>;

	static_assert(ADLO_UnaryBase<T1>::_type1::value == _type2::value, "Non matching ADLO types.");

	inline ADLO_BinaryBase(const T1& arg1, const T2& arg2)
		: ADLO_UnaryBase<T1>(arg1)
		, mArg2(arg2)
	{
	}

protected:
	T2 mArg2;
};

template <typename T1, typename T2>
class ADLO_BinaryValueBase : public ADLO_UnaryBase<T1> {
public:
	using typename ADLO_UnaryBase<T1>::value_t;
	using typename ADLO_UnaryBase<T1>::is_scalar;

	inline ADLO_BinaryValueBase(const T1& arg1, T2 arg2)
		: ADLO_UnaryBase<T1>(arg1)
		, mArg2(arg2)
	{
	}

protected:
	T2 mArg2;
};

template <typename T1, typename T2>
class ADLO_Add : public ADLO_BinaryBase<T1, T2> {
public:
	using ADLO_BinaryBase<T1, T2>::ADLO_BinaryBase;

	inline typename ADLO_BinaryBase<T1, T2>::value_t v() const;
	inline typename ADLO_BinaryBase<T1, T2>::value_t d(uint32 i) const;
};

template <typename T1>
class ADLO_Neg : public ADLO_UnaryBase<T1> {
public:
	using ADLO_UnaryBase<T1>::ADLO_UnaryBase;

	inline typename ADLO_UnaryBase<T1>::value_t v() const;
	inline typename ADLO_UnaryBase<T1>::value_t d(uint32 i) const;
};

template <typename T1, typename T2>
class ADLO_Sub : public ADLO_BinaryBase<T1, T2> {
public:
	using ADLO_BinaryBase<T1, T2>::ADLO_BinaryBase;

	inline typename ADLO_BinaryBase<T1, T2>::value_t v() const;
	inline typename ADLO_BinaryBase<T1, T2>::value_t d(uint32 i) const;
};

template <typename T1, typename T2>
class ADLO_Scale : public ADLO_BinaryValueBase<T1, T2> {
public:
	using ADLO_BinaryValueBase<T1, T2>::ADLO_BinaryValueBase;

	inline typename ADLO_BinaryValueBase<T1, T2>::value_t v() const;
	inline typename ADLO_BinaryValueBase<T1, T2>::value_t d(uint32 i) const;
};

template <typename T1, typename T2>
class ADLO_InvScale : public ADLO_BinaryValueBase<T1, T2> {
public:
	using ADLO_BinaryValueBase<T1, T2>::ADLO_BinaryValueBase;

	template <typename U1 = T1, typename U2 = T2>
	inline std::enable_if_t<ADLO_BinaryValueBase<U1, U2>::is_scalar::value, typename ADLO_BinaryValueBase<U1, U2>::value_t> v() const;
	template <typename U1 = T1, typename U2 = T2>
	inline std::enable_if_t<!ADLO_BinaryValueBase<U1, U2>::is_scalar::value, typename ADLO_BinaryValueBase<U1, U2>::value_t> v() const;

	template <typename U1 = T1, typename U2 = T2>
	inline std::enable_if_t<ADLO_BinaryValueBase<U1, U2>::is_scalar::value, typename ADLO_BinaryValueBase<U1, U2>::value_t> d(uint32 i) const;
	template <typename U1 = T1, typename U2 = T2>
	inline std::enable_if_t<!ADLO_BinaryValueBase<U1, U2>::is_scalar::value, typename ADLO_BinaryValueBase<U1, U2>::value_t> d(uint32 i) const;
};

template <typename T1, typename T2>
class ADLO_Mul : public ADLO_BinaryBase<T1, T2> {
public:
	using ADLO_BinaryBase<T1, T2>::ADLO_BinaryBase;

	template <typename U1 = T1, typename U2 = T2>
	inline std::enable_if_t<ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t> v() const;
	template <typename U1 = T1, typename U2 = T2>
	inline std::enable_if_t<!ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t> v() const;

	template <typename U1 = T1, typename U2 = T2>
	inline std::enable_if_t<ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t> d(uint32 i) const;
	template <typename U1 = T1, typename U2 = T2>
	inline std::enable_if_t<!ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t> d(uint32 i) const;
};

template <typename T1, typename T2>
class ADLO_Div : public ADLO_BinaryBase<T1, T2> {
public:
	using ADLO_BinaryBase<T1, T2>::ADLO_BinaryBase;

	template <typename U1 = T1, typename U2 = T2>
	inline std::enable_if_t<ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t> v() const;
	template <typename U1 = T1, typename U2 = T2>
	inline std::enable_if_t<!ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t> v() const;

	template <typename U1 = T1, typename U2 = T2>
	inline std::enable_if_t<ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t> d(uint32 i) const;
	template <typename U1 = T1, typename U2 = T2>
	inline std::enable_if_t<!ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t> d(uint32 i) const;
};

template <typename T1>
class ADLO_Abs : public ADLO_UnaryBase<T1> {
public:
	using ADLO_UnaryBase<T1>::ADLO_UnaryBase;

	template <typename U1 = T1>
	inline std::enable_if_t<ADLO_UnaryBase<U1>::is_scalar::value, typename ADLO_UnaryBase<U1>::value_t> v() const;
	template <typename U1 = T1>
	inline std::enable_if_t<!ADLO_UnaryBase<U1>::is_scalar::value, typename ADLO_UnaryBase<U1>::value_t> v() const;

	template <typename U1 = T1>
	inline std::enable_if_t<ADLO_UnaryBase<U1>::is_scalar::value, typename ADLO_UnaryBase<U1>::value_t> d(uint32 i) const;
	template <typename U1 = T1>
	inline std::enable_if_t<!ADLO_UnaryBase<U1>::is_scalar::value, typename ADLO_UnaryBase<U1>::value_t> d(uint32 i) const;
};

// Traits
template <typename T>
using is_adlo = std::is_base_of<ADLO_Base, T>;

template <typename T>
using is_adlo_scalar = std::integral_constant<bool, (is_adlo<T>::value && T::_type::value == 0) || is_ad_scalar<T>::value>;

template <typename T>
using is_adlo_vector = std::integral_constant<bool, (is_adlo<T>::value && T::_type::value == 1) || is_ad_vector<T>::value>;

template <typename T1, typename T2, typename RetT>
using enable_if_adlo_scalar_t = std::enable_if_t<PR::Lazy::is_adlo_scalar<T1>::value && PR::Lazy::is_adlo_scalar<T2>::value, RetT>;

template <typename T1, typename RetT>
using enable_if_adlo_scalar1_t = std::enable_if_t<PR::Lazy::is_adlo_scalar<T1>::value, RetT>;

template <typename T1, typename T2, typename RetT>
using enable_if_adlo_vector_t = std::enable_if_t<PR::Lazy::is_adlo_vector<T1>::value && PR::Lazy::is_adlo_vector<T2>::value, RetT>;

template <typename T1, typename RetT>
using enable_if_adlo_vector1_t = std::enable_if_t<PR::Lazy::is_adlo_vector<T1>::value, RetT>;

template <typename T1, typename T2, typename RetT>
using enable_if_adlo_op_t = std::enable_if_t<(PR::Lazy::is_adlo_scalar<T1>::value && PR::Lazy::is_adlo_scalar<T2>::value) || (PR::Lazy::is_adlo_vector<T1>::value && PR::Lazy::is_adlo_vector<T2>::value), RetT>;

template <typename T1, typename RetT>
using enable_if_adlo_op1_t = std::enable_if_t<PR::Lazy::is_adlo_scalar<T1>::value || PR::Lazy::is_adlo_vector<T1>::value, RetT>;
}

template <typename T, size_t P>
class AutoDiff_Base : public AutoDiff_CoreBase {
public:
	using value_t = std::remove_cv_t<std::decay_t<T>>;
	using array_t = std::array<value_t, P>;

	AutoDiff_Base()						= default;
	AutoDiff_Base(const AutoDiff_Base&) = default;
	AutoDiff_Base(AutoDiff_Base&&)		= default;
	AutoDiff_Base& operator=(const AutoDiff_Base&) = default;
	AutoDiff_Base& operator=(AutoDiff_Base&&) = default;

	AutoDiff_Base(const value_t v, const array_t& l);
	template <typename... Args>
	AutoDiff_Base(const value_t v, const Args&... gs);

	template <typename ADLO>
	AutoDiff_Base(const ADLO& adlo, std::enable_if_t<Lazy::is_adlo<ADLO>::value>* = 0);
	template <typename ADLO>
	std::enable_if_t<Lazy::is_adlo<ADLO>::value, AutoDiff_Base&>
	operator=(const ADLO& adlo);

	// Accessor
	inline const value_t& v() const;
	inline value_t& v();

	inline const value_t& d(size_t p) const;
	inline value_t& d(size_t p);

protected:
	value_t mValue;
	array_t mGradients;
};

// Scalar
template <typename T, size_t P>
class AutoDiff_Scalar : public AutoDiff_Base<T, P> {
public:
	using base_t = AutoDiff_Base<T, P>;
	using typename base_t::value_t;
	using scalar_t = typename base_t::value_t;

	using _type = std::integral_constant<int, 0>;

	static_assert(std::is_arithmetic<value_t>::value, "Expected scalar for AutoDiff_Scalar");

	using self_t		= AutoDiff_Scalar<value_t, P>;
	using InvReturnType = Lazy::ADLO_InvScale<self_t, scalar_t>;
	using AbsReturnType = Lazy::ADLO_Abs<self_t>;

	using base_t::AutoDiff_Base;

	// Operators
	template <typename U>
	inline Lazy::enable_if_adlo_scalar1_t<U, AutoDiff_Scalar<T, P>&> operator+=(const U& other);
	template <typename U>
	inline Lazy::enable_if_adlo_scalar1_t<U, AutoDiff_Scalar<T, P>&> operator-=(const U& other);

	template <typename U>
	inline Lazy::enable_if_adlo_scalar1_t<U, AutoDiff_Scalar<T, P>&> operator*=(const U& other);
	inline AutoDiff_Scalar<T, P>& operator*=(const scalar_t& other);

	template <typename U>
	inline Lazy::enable_if_adlo_scalar1_t<U, AutoDiff_Scalar<T, P>&> operator/=(const U& other);
	inline AutoDiff_Scalar<T, P>& operator/=(const scalar_t& other);

	inline InvReturnType inv() const;
	inline AbsReturnType abs() const;

	template <typename U, typename dU>
	inline void applyUnaryExpr(U expr, dU dexpr);
	template <typename U, typename dU>
	inline self_t unaryExpr(U expr, dU dexpr) const;

	template <typename T2, typename B, typename dB1, typename dB2>
	inline Lazy::enable_if_adlo_scalar1_t<T2, void> applyBinaryExpr(const T2& other, B expr, dB1 dexpr1, dB2 dexpr2);
	template <typename T2, typename B, typename dB1, typename dB2>
	inline Lazy::enable_if_adlo_scalar1_t<T2, self_t> binaryExpr(const T2& other, B expr, dB1 dexpr1, dB2 dexpr2) const;
};

// Some standard operations
template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> sqrt(const T& v);
template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> cbrt(const T& v);
template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> sin(const T& v);
template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> asin(const T& v);
template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> cos(const T& v);
template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> acos(const T& v);
template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> tan(const T& v);
template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> atan(const T& v);
template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> exp(const T& v);
template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> log(const T& v);

// Vector
template <typename T, size_t D, size_t P>
class AutoDiff_Vector : public AutoDiff_Base<Eigen::Matrix<T, D, 1>, P> {
public:
	using base_t = AutoDiff_Base<Eigen::Matrix<T, D, 1>, P>;
	using typename base_t::value_t;
	using scalar_t = std::remove_cv_t<std::decay_t<T>>;

	using _type = std::integral_constant<int, 1>;

	static_assert(std::is_arithmetic<scalar_t>::value, "Expected T to be scalar for AutoDiff_Vector3");

	using self_t	 = AutoDiff_Vector<scalar_t, D, P>;
	using scalar_v_t = AutoDiff_Scalar<scalar_t, P>;

	using InvReturnType   = Lazy::ADLO_InvScale<self_t, scalar_t>;
	using AbsReturnType   = Lazy::ADLO_Abs<self_t>;
	using DotReturnType   = typename self_t::scalar_v_t;
	using CrossReturnType = typename self_t::self_t;

	using base_t::AutoDiff_Base;

	// Operators
	template <typename U>
	inline Lazy::enable_if_adlo_vector1_t<U, AutoDiff_Vector<T, D, P>&> operator+=(const U& other);
	template <typename U>
	inline Lazy::enable_if_adlo_vector1_t<U, AutoDiff_Vector<T, D, P>&> operator-=(const U& other);

	template <typename U>
	inline Lazy::enable_if_adlo_vector1_t<U, AutoDiff_Vector<T, D, P>&> operator*=(const U& other);
	inline AutoDiff_Vector<T, D, P>& operator*=(const scalar_t& other);

	template <typename U>
	inline Lazy::enable_if_adlo_vector1_t<U, AutoDiff_Vector<T, D, P>&> operator/=(const U& other);
	inline AutoDiff_Vector<T, D, P>& operator/=(const scalar_t& other);

	inline InvReturnType inv() const;
	inline AbsReturnType abs() const;

	template <typename U>
	inline Lazy::enable_if_adlo_vector1_t<U, DotReturnType> dot(const U& other) const;
	template <typename U>
	inline Lazy::enable_if_adlo_vector1_t<U, CrossReturnType> cross(const U& other) const;

	inline scalar_v_t norm() const;
	inline void normalize();
	inline self_t normalized() const;
};

template <typename Scalar, typename T, size_t D, size_t P>
inline std::enable_if_t<std::is_arithmetic<Scalar>::value, AutoDiff_Vector<T, D, P>>
operator*(Scalar f, const AutoDiff_Vector<T, D, P>& other);

template <typename Scalar, typename T, size_t D, size_t P>
inline std::enable_if_t<std::is_arithmetic<Scalar>::value, AutoDiff_Vector<T, D, P>>
operator/(Scalar f, const AutoDiff_Vector<T, D, P>& other);

// General operators
template <typename T1, typename T2>
inline Lazy::enable_if_adlo_op_t<T1, T2, Lazy::ADLO_Add<T1, T2>>
operator+(const T1& v1, const T2& v2);

template <typename T1>
inline Lazy::enable_if_adlo_op1_t<T1, Lazy::ADLO_Neg<T1>>
operator-(const T1& v1);

template <typename T1, typename T2>
inline Lazy::enable_if_adlo_op_t<T1, T2, Lazy::ADLO_Sub<T1, T2>>
operator-(const T1& v1, const T2& v2);

template <typename T1, typename T2>
inline Lazy::enable_if_adlo_op_t<T1, T2, Lazy::ADLO_Mul<T1, T2>>
operator*(const T1& v1, const T2& v2);

template <typename T1>
inline Lazy::enable_if_adlo_op1_t<T1, Lazy::ADLO_Scale<T1, typename T1::value_t>>
operator*(const T1& v1, typename T1::scalar_t v2);

template <typename T1>
inline Lazy::enable_if_adlo_op1_t<T1, Lazy::ADLO_Scale<T1, typename T1::value_t>>
operator*(typename T1::scalar_t v2, const T1& v1);

template <typename T1, typename T2>
inline Lazy::enable_if_adlo_op_t<T1, T2, Lazy::ADLO_Div<T1, T2>>
operator/(const T1& v1, const T2& v2);

template <typename T1>
inline Lazy::enable_if_adlo_op1_t<T1, Lazy::ADLO_Scale<T1, typename T1::value_t>>
operator/(const T1& v1, typename T1::scalar_t v2);

template <typename T1>
inline Lazy::enable_if_adlo_op1_t<T1, Lazy::ADLO_InvScale<T1, typename T1::value_t>>
operator/(typename T1::scalar_t v2, const T1& v1);

// Types
typedef AutoDiff_Scalar<float, 1> DxF;
typedef AutoDiff_Scalar<float, 2> DxDyF;
typedef AutoDiff_Vector<float, 3, 1> DxV3F;
typedef AutoDiff_Vector<float, 3, 2> DxDyV3F;
}

#include "AutoDiff.inl"
#include "AutoDiff_ADLO.inl"
#include "AutoDiff_Operators.inl"
#include "AutoDiff_Scalar.inl"
#include "AutoDiff_Vector.inl"