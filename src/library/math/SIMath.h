#pragma once

#include <cmath>
#include <ratio>
#include <type_traits>

/*
 * Limited and basic si units based math expression header only library for C++11,
 * to ensure "failure proof" coding of scientific equations, especially in the CG area.
 * 
 * For a more complete and feature rich library visit the (C++14) 'units' library developed by nholthaus:
 * https://github.com/nholthaus/units
 */
namespace SI {

#ifndef SIMATH_DEFAULT_BASE
#define SIMATH_DEFAULT_BASE float
#endif

#ifndef SIMATH_BASE_POSTFIX
#define SIMATH_BASE_POSTFIX U
#endif

#ifndef SIMATH_COORDINATE_POSTFIX
#define SIMATH_COORDINATE_POSTFIX C
#endif

#ifndef SIMATH_STANDARD_POSTFIX
#define SIMATH_STANDARD_POSTFIX
#endif

#define _SIMATH_UAT(S1, S2) S1##S2
#define _SIMATH_UAT2(S1, S2) _SIMATH_UAT(S1, S2)
#define _SIMATH_U(Unit) _SIMATH_UAT2(Unit, SIMATH_BASE_POSTFIX)
#define _SIMATH_C(Unit) _SIMATH_UAT2(Unit, SIMATH_COORDINATE_POSTFIX)
#define _SIMATH_S(Unit) _SIMATH_UAT2(Unit, SIMATH_STANDARD_POSTFIX)

struct SIBaseClass {
};

// -----------------------------
template <typename Base, int Coordinate, class L, class M, class T, class I, class K, class N, class J, class R>
struct SIBase : public SIBaseClass {
	typedef Base base_t;
	static constexpr int coordinate_system = Coordinate;
	typedef L length_dim;
	typedef M mass_dim;
	typedef T time_dim;
	typedef I current_dim;
	typedef K temperature_dim;
	typedef N amount_dim;
	typedef J luminous_intensity_dim;
	typedef R radian_dim;

	typedef std::ratio_add<L,
						   std::ratio_add<M,
										  std::ratio_add<T,
														 std::ratio_add<I,
																		std::ratio_add<K,
																					   std::ratio_add<N,
																									  std::ratio_add<J, R>>>>>>>
		full_dim;

	Base Value;

	SIBase() = default;
	SIBase(const Base& val)
		: Value(val)
	{
	}

	SIBase(const SIBase& copy) = default;
	SIBase(SIBase&& copy)	  = default;

	SIBase& operator=(const SIBase& copy) = default;
	SIBase& operator=(SIBase&& copy) = default;

	explicit operator Base() const { return Value; }
};

static_assert(std::is_pod<SIBase<SIMATH_DEFAULT_BASE, 0,
								 std::ratio<0>, std::ratio<0>, std::ratio<0>, std::ratio<0>, std::ratio<0>,
								 std::ratio<0>, std::ratio<0>, std::ratio<0>>>::value,
			  "SIBase class is not POD conform... This is fatal. Try another SIMATH_DEFAULT_BASE or contact author if you didn't redefined SIMATH_DEFAULT_BASE");

// -----------------------------

// -----------------------------
// Basic traits
namespace traits {
template <class Unit>
struct is_unit : public std::is_base_of<SIBaseClass, Unit> {
};

template <class Unit, typename = typename std::enable_if<is_unit<Unit>::value>::type>
struct has_length : public std::ratio_not_equal<typename Unit::length_dim, std::ratio<0, 1>> {
};
template <class Unit, typename = typename std::enable_if<is_unit<Unit>::value>::type>
struct has_mass : public std::ratio_not_equal<typename Unit::mass_dim, std::ratio<0, 1>> {
};
template <class Unit, typename = typename std::enable_if<is_unit<Unit>::value>::type>
struct has_time : public std::ratio_not_equal<typename Unit::time_dim, std::ratio<0, 1>> {
};
template <class Unit, typename = typename std::enable_if<is_unit<Unit>::value>::type>
struct has_current : public std::ratio_not_equal<typename Unit::current_dim, std::ratio<0, 1>> {
};
template <class Unit, typename = typename std::enable_if<is_unit<Unit>::value>::type>
struct has_temperature : public std::ratio_not_equal<typename Unit::temperature_dim, std::ratio<0, 1>> {
};
template <class Unit, typename = typename std::enable_if<is_unit<Unit>::value>::type>
struct has_amount : public std::ratio_not_equal<typename Unit::amount_dim, std::ratio<0, 1>> {
};
template <class Unit, typename = typename std::enable_if<is_unit<Unit>::value>::type>
struct has_luminous_intensity : public std::ratio_not_equal<typename Unit::luminous_intensity_dim, std::ratio<0, 1>> {
};
template <class Unit, typename = typename std::enable_if<is_unit<Unit>::value>::type>
struct has_radian : public std::ratio_not_equal<typename Unit::radian_dim, std::ratio<0, 1>> {
};

template <class Unit, typename = typename std::enable_if<is_unit<Unit>::value>::type>
struct is_scalar : public std::integral_constant<bool, !(has_length<Unit>::value || has_mass<Unit>::value || has_time<Unit>::value || has_current<Unit>::value || has_temperature<Unit>::value || has_amount<Unit>::value || has_luminous_intensity<Unit>::value || has_radian<Unit>::value)> {
};

template <class Unit, typename = typename std::enable_if<is_unit<Unit>::value>::type>
struct is_transcendental : public std::integral_constant<bool, is_scalar<Unit>::value> {
};
} // namespace traits

namespace details {
template <class Unit1, class Unit2, typename = typename std::enable_if<traits::is_unit<Unit1>::value && traits::is_unit<Unit2>::value>::type>
struct unit_op {
	static_assert(std::is_same<typename Unit1::base_t, typename Unit2::base_t>::value,
				  "Unit 1 and Unit 2 have not the same underlying base");
	static_assert(Unit1::coordinate_system == Unit2::coordinate_system,
				  "Unit 1 and Unit 2 have not the same underlying coordinate system");
};

template <class Unit1, class Unit2, typename = typename std::enable_if<traits::is_unit<Unit1>::value && traits::is_unit<Unit2>::value>::type>
struct unit_mul : public unit_op<Unit1, Unit2> {
	typedef SIBase<typename Unit1::base_t, Unit1::coordinate_system,
				   std::ratio_add<typename Unit1::length_dim, typename Unit2::length_dim>,
				   std::ratio_add<typename Unit1::mass_dim, typename Unit2::mass_dim>,
				   std::ratio_add<typename Unit1::time_dim, typename Unit2::time_dim>,
				   std::ratio_add<typename Unit1::current_dim, typename Unit2::current_dim>,
				   std::ratio_add<typename Unit1::temperature_dim, typename Unit2::temperature_dim>,
				   std::ratio_add<typename Unit1::amount_dim, typename Unit2::amount_dim>,
				   std::ratio_add<typename Unit1::luminous_intensity_dim, typename Unit2::luminous_intensity_dim>,
				   std::ratio_add<typename Unit1::radian_dim, typename Unit2::radian_dim>>
		type;
};

template <class Unit1, class Unit2, typename = typename std::enable_if<traits::is_unit<Unit1>::value && traits::is_unit<Unit2>::value>::type>
struct unit_div : public unit_op<Unit1, Unit2> {
	typedef SIBase<typename Unit1::base_t, Unit1::coordinate_system,
				   std::ratio_subtract<typename Unit1::length_dim, typename Unit2::length_dim>,
				   std::ratio_subtract<typename Unit1::mass_dim, typename Unit2::mass_dim>,
				   std::ratio_subtract<typename Unit1::time_dim, typename Unit2::time_dim>,
				   std::ratio_subtract<typename Unit1::current_dim, typename Unit2::current_dim>,
				   std::ratio_subtract<typename Unit1::temperature_dim, typename Unit2::temperature_dim>,
				   std::ratio_subtract<typename Unit1::amount_dim, typename Unit2::amount_dim>,
				   std::ratio_subtract<typename Unit1::luminous_intensity_dim, typename Unit2::luminous_intensity_dim>,
				   std::ratio_subtract<typename Unit1::radian_dim, typename Unit2::radian_dim>>
		type;
};

template <class Unit, class Exp, typename = typename std::enable_if<traits::is_unit<Unit>::value>::type>
struct unit_pow {
	typedef SIBase<typename Unit::base_t, Unit::coordinate_system,
				   std::ratio_multiply<typename Unit::length_dim, Exp>,
				   std::ratio_multiply<typename Unit::mass_dim, Exp>,
				   std::ratio_multiply<typename Unit::time_dim, Exp>,
				   std::ratio_multiply<typename Unit::current_dim, Exp>,
				   std::ratio_multiply<typename Unit::temperature_dim, Exp>,
				   std::ratio_multiply<typename Unit::amount_dim, Exp>,
				   std::ratio_multiply<typename Unit::luminous_intensity_dim, Exp>,
				   std::ratio_multiply<typename Unit::radian_dim, Exp>>
		type;
};

template <class Unit, typename = typename std::enable_if<traits::is_unit<Unit>::value>::type>
struct unit_sqrt {
	typedef typename unit_pow<Unit, std::ratio<1, 2>>::type type;
};

template <class Unit, typename = typename std::enable_if<traits::is_unit<Unit>::value>::type>
struct unit_inv {
	typedef typename unit_pow<Unit, std::ratio<-1>>::type type;
};
} // namespace details

// ------------------------------------
#define _SIMATH_DEFINE_UNIT_SET(Name, L, M, T, I, K, N, J, R)                                                                                                                 \
	template <typename Base = SIMATH_DEFAULT_BASE, int Coordinate = 0>                                                                                                        \
	using _SIMATH_U(Name) = SIBase<Base, Coordinate, std::ratio<L>, std::ratio<M>, std::ratio<T>, std::ratio<I>, std::ratio<K>, std::ratio<N>, std::ratio<J>, std::ratio<R>>; \
	template <int Coordinate = 0>                                                                                                                                             \
	using _SIMATH_C(Name) = _SIMATH_U(Name)<SIMATH_DEFAULT_BASE, Coordinate>;                                                                                                 \
	using _SIMATH_S(Name) = _SIMATH_C(Name)<0>

#define _SIMATH_DEFINE_UNIT_MUL(Name, Unit1, Unit2)                                          \
	template <typename Base = SIMATH_DEFAULT_BASE, int Coordinate = 0>                       \
	using _SIMATH_U(Name) = typename details::unit_mul<_SIMATH_U(Unit1) < Base, Coordinate>, \
		  _SIMATH_U(Unit2)<Base, Coordinate>> ::type;                                        \
	template <int Coordinate = 0>                                                            \
	using _SIMATH_C(Name) = _SIMATH_U(Name)<SIMATH_DEFAULT_BASE, Coordinate>;                \
	using _SIMATH_S(Name) = _SIMATH_C(Name)<0>

#define _SIMATH_DEFINE_UNIT_DIV(Name, Unit1, Unit2)                                          \
	template <typename Base = SIMATH_DEFAULT_BASE, int Coordinate = 0>                       \
	using _SIMATH_U(Name) = typename details::unit_div<_SIMATH_U(Unit1) < Base, Coordinate>, \
		  _SIMATH_U(Unit2)<Base, Coordinate>> ::type;                                        \
	template <int Coordinate = 0>                                                            \
	using _SIMATH_C(Name) = _SIMATH_U(Name)<SIMATH_DEFAULT_BASE, Coordinate>;                \
	using _SIMATH_S(Name) = _SIMATH_C(Name)<0>

#define _SIMATH_DEFINE_UNIT_INV(Name, Unit)                                                         \
	template <typename Base = SIMATH_DEFAULT_BASE, int Coordinate = 0>                              \
	using _SIMATH_U(Name) = typename details::unit_inv<_SIMATH_U(Unit) < Base, Coordinate>> ::type; \
	template <int Coordinate = 0>                                                                   \
	using _SIMATH_C(Name) = _SIMATH_U(Name)<SIMATH_DEFAULT_BASE, Coordinate>;                       \
	using _SIMATH_S(Name) = _SIMATH_C(Name)<0>

// -----------------------------
// Base units
//                          L M T I K N J R
_SIMATH_DEFINE_UNIT_SET(Scalar, 0, 0, 0, 0, 0, 0, 0, 0);
_SIMATH_DEFINE_UNIT_SET(Length, 1, 0, 0, 0, 0, 0, 0, 0);
_SIMATH_DEFINE_UNIT_SET(Mass, 0, 1, 0, 0, 0, 0, 0, 0);
_SIMATH_DEFINE_UNIT_SET(Time, 0, 0, 1, 0, 0, 0, 0, 0);
_SIMATH_DEFINE_UNIT_SET(Current, 0, 0, 0, 1, 0, 0, 0, 0);
_SIMATH_DEFINE_UNIT_SET(Temperature, 0, 0, 0, 0, 1, 0, 0, 0);
_SIMATH_DEFINE_UNIT_SET(Amount, 0, 0, 0, 0, 0, 1, 0, 0);
_SIMATH_DEFINE_UNIT_SET(LuminousIntensity, 0, 0, 0, 0, 0, 0, 1, 0);
_SIMATH_DEFINE_UNIT_SET(Radian, 0, 0, 0, 0, 0, 0, 0, 1); // Not really an official SI base unit, but it's practical to define it that way

// -----------------------------
// Derived Units
_SIMATH_DEFINE_UNIT_MUL(Area, Length, Length);
_SIMATH_DEFINE_UNIT_MUL(Volume, Area, Length);
_SIMATH_DEFINE_UNIT_MUL(Steradian, Radian, Radian);
_SIMATH_DEFINE_UNIT_INV(Frequency, Time);
_SIMATH_DEFINE_UNIT_SET(Force, 1, 1, -2, 0, 0, 0, 0, 0);
_SIMATH_DEFINE_UNIT_DIV(Pressure, Force, Area);
_SIMATH_DEFINE_UNIT_MUL(Energy, Force, Length);
_SIMATH_DEFINE_UNIT_DIV(Power, Energy, Time);
_SIMATH_DEFINE_UNIT_MUL(Charge, Time, Current);
_SIMATH_DEFINE_UNIT_DIV(Voltage, Power, Current);
_SIMATH_DEFINE_UNIT_DIV(Capacitance, Charge, Voltage);
_SIMATH_DEFINE_UNIT_DIV(Resistance, Voltage, Current);
_SIMATH_DEFINE_UNIT_DIV(Conductance, Current, Voltage);
_SIMATH_DEFINE_UNIT_MUL(MagneticFlux, Voltage, Time);
_SIMATH_DEFINE_UNIT_DIV(MagneticFluxDensity, MagneticFlux, Area);
_SIMATH_DEFINE_UNIT_DIV(Inductance, MagneticFlux, Current);
// Radiometric
_SIMATH_DEFINE_UNIT_DIV(SpectralFluxFrequency, Power, Frequency);
_SIMATH_DEFINE_UNIT_DIV(SpectralFluxWavelength, Power, Length);
_SIMATH_DEFINE_UNIT_DIV(RadiantIntensity, Power, Steradian);
_SIMATH_DEFINE_UNIT_DIV(SpectralIntensityFrequency, RadiantIntensity, Frequency);
_SIMATH_DEFINE_UNIT_DIV(SpectralIntensityWavelength, RadiantIntensity, Length);
_SIMATH_DEFINE_UNIT_DIV(Radiance, RadiantIntensity, Area);
_SIMATH_DEFINE_UNIT_DIV(SpectralRadianceFrequency, Radiance, Frequency);
_SIMATH_DEFINE_UNIT_DIV(SpectralRadianceWavelength, Radiance, Length);
_SIMATH_DEFINE_UNIT_DIV(Irradiance, Power, Area);
_SIMATH_DEFINE_UNIT_DIV(SpectralIrradianceFrequency, Irradiance, Frequency);
_SIMATH_DEFINE_UNIT_DIV(SpectralIrradianceWavelength, Irradiance, Length);
// Photometric
_SIMATH_DEFINE_UNIT_MUL(LuminousFlux, LuminousIntensity, Steradian);
_SIMATH_DEFINE_UNIT_MUL(LuminousEnergy, LuminousFlux, Time);
_SIMATH_DEFINE_UNIT_DIV(Luminance, LuminousIntensity, Area);
_SIMATH_DEFINE_UNIT_DIV(Illuminance, LuminousFlux, Area);
_SIMATH_DEFINE_UNIT_MUL(LuminousExposure, Illuminance, Time);

// -----------------------------
// Operations
template <typename Unit>
inline typename std::enable_if<SI::traits::is_unit<Unit>::value, Unit>::type
operator+(const Unit& v1, const Unit& v2)
{
	return v1.Value + v2.Value;
}

template <typename Unit>
inline typename std::enable_if<SI::traits::is_unit<Unit>::value, Unit>::type
operator-(const Unit& v1, const Unit& v2)
{
	return v1.Value - v2.Value;
}

template <typename Unit1, typename Unit2>
inline typename std::enable_if<SI::traits::is_unit<Unit1>::value && SI::traits::is_unit<Unit2>::value, typename SI::details::unit_mul<Unit1, Unit2>::type>::type
operator*(const Unit1& v1, const Unit2& v2)
{
	return v1.Value * v2.Value;
}

template <typename Unit>
inline typename std::enable_if<SI::traits::is_unit<Unit>::value, Unit>::type
operator*(const Unit& v, const typename Unit::base_t& b)
{
	return v.Value * b;
}

template <typename Unit>
inline typename std::enable_if<SI::traits::is_unit<Unit>::value, Unit>::type
operator*(const typename Unit::base_t& b, const Unit& v)
{
	return b * v.Value;
}

template <typename Unit1, typename Unit2>
inline typename std::enable_if<SI::traits::is_unit<Unit1>::value && SI::traits::is_unit<Unit2>::value, typename SI::details::unit_div<Unit1, Unit2>::type>::type
operator/(const Unit1& v1, const Unit2& v2)
{
	return v1.Value / v2.Value;
}

template <typename Unit>
inline typename std::enable_if<SI::traits::is_unit<Unit>::value, Unit>::type
operator/(const Unit& v, const typename Unit::base_t& b)
{
	return v.Value / b;
}

template <typename Unit>
inline typename std::enable_if<SI::traits::is_unit<Unit>::value, typename SI::details::unit_inv<Unit>::type>::type
operator/(const typename Unit::base_t& b, const Unit& v)
{
	return b / v.Value;
}
}
