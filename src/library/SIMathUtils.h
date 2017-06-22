#pragma once
#include "SIMath.h"

#include <sstream>

namespace SI {
template <class Ratio>
inline std::string ratio_to_string()
{
	std::stringstream stream;
	stream << Ratio::num;
	if (Ratio::den != 1)
		stream << "/" << Ratio::den;

	return stream.str();
}

template <class Unit>
inline typename std::enable_if<traits::is_unit<Unit>::value, std::string>::type unit_to_string()
{
	std::stringstream stream;
	if (traits::has_length<Unit>::value)
		stream << "m(" << ratio_to_string<typename Unit::length_dim>() << ")";
	if (traits::has_mass<Unit>::value)
		stream << "kg(" << ratio_to_string<typename Unit::mass_dim>() << ")";
	if (traits::has_time<Unit>::value)
		stream << "s(" << ratio_to_string<typename Unit::time_dim>() << ")";
	if (traits::has_current<Unit>::value)
		stream << "A(" << ratio_to_string<typename Unit::current_dim>() << ")";
	if (traits::has_temperature<Unit>::value)
		stream << "K(" << ratio_to_string<typename Unit::temperature_dim>() << ")";
	if (traits::has_amount<Unit>::value)
		stream << "mol(" << ratio_to_string<typename Unit::amount_dim>() << ")";
	if (traits::has_luminous_intensity<Unit>::value)
		stream << "cd(" << ratio_to_string<typename Unit::luminous_intensity_dim>() << ")";
	if (traits::has_radian<Unit>::value)
		stream << "rad(" << ratio_to_string<typename Unit::radian_dim>() << ")";

	return stream.str();
}

// IO Stream (only output)
template <class Unit>
inline typename std::enable_if<SI::traits::is_unit<Unit>::value && !SI::traits::is_scalar<Unit>::value, std::ostream&>::type operator<<(std::ostream& os, const Unit& obj)
{
	os << obj.Value << " " << SI::unit_to_string<Unit>();
	return os;
}

template <class Unit>
inline typename std::enable_if<SI::traits::is_unit<Unit>::value && SI::traits::is_scalar<Unit>::value, std::ostream&>::type operator<<(std::ostream& os, const Unit& obj)
{
	os << obj.Value;
	return os;
}
}