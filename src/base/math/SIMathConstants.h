#pragma once
#include "SIMath.h"

namespace SI {
namespace constants {

template <typename Base = SIMATH_DEFAULT_BASE, int Coordinate = 0>
inline constexpr _SIMATH_U(Scalar)<Base, Coordinate> pi()
{
	return 3.14159265358979323846;
}

template <typename Base = SIMATH_DEFAULT_BASE, int Coordinate = 0>
inline constexpr _SIMATH_U(Scalar)<Base, Coordinate> e()
{
	return 2.71828182845904523536;
}

/* Speed of light in vacuum*/
template <typename Base = SIMATH_DEFAULT_BASE, int Coordinate = 0>
inline constexpr SIBase<Base, Coordinate, std::ratio<1>, std::ratio<0>, std::ratio<-1>, std::ratio<0>, std::ratio<0>, std::ratio<0>, std::ratio<0>, std::ratio<0>> c()
{
	return 299792458;
}

/* Earth gravitation */
template <typename Base = SIMATH_DEFAULT_BASE, int Coordinate = 0>
inline constexpr SIBase<Base, Coordinate, std::ratio<3>, std::ratio<-1>, std::ratio<-2>, std::ratio<0>, std::ratio<0>, std::ratio<0>, std::ratio<0>, std::ratio<0>> G()
{
	return 6.6740831;
}

/* Plank constant */
template <typename Base = SIMATH_DEFAULT_BASE, int Coordinate = 0>
inline constexpr SIBase<Base, Coordinate, std::ratio<2>, std::ratio<1>, std::ratio<-1>, std::ratio<0>, std::ratio<0>, std::ratio<0>, std::ratio<0>, std::ratio<0>> h()
{
	return 6.62607004081e-34;
}

/* Elemental charge */
template <typename Base = SIMATH_DEFAULT_BASE, int Coordinate = 0>
inline constexpr _SIMATH_U(Charge)<Base, Coordinate> ec()
{
	return 1.602176620898e-19;
}

/* electron mass */
template <typename Base = SIMATH_DEFAULT_BASE, int Coordinate = 0>
inline constexpr _SIMATH_U(Mass)<Base, Coordinate> me()
{
	return 9.1093835611e-31;
}

/* boltzmann constant */
template <typename Base = SIMATH_DEFAULT_BASE, int Coordinate = 0>
inline constexpr SIBase<Base, Coordinate, std::ratio<2>, std::ratio<1>, std::ratio<-2>, std::ratio<0>, std::ratio<-1>, std::ratio<0>, std::ratio<0>, std::ratio<0>> kb()
{
	return 1.3806485279e-23;
}
} // namespace constants
} // namespace SI