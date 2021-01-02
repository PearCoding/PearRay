#include "Parameter.h"

namespace PR {

Parameter Parameter::fromBool(bool v)
{
	return Parameter(ParameterType::Bool, { v });
}

Parameter Parameter::fromInt(Int v)
{
	return Parameter(ParameterType::Int, { v });
}

Parameter Parameter::fromUInt(UInt v)
{
	return Parameter(ParameterType::UInt, { v });
}

Parameter Parameter::fromNumber(Number v)
{
	return Parameter(ParameterType::Number, { v });
}

Parameter Parameter::fromString(const std::string& v)
{
	return Parameter(ParameterType::String, { v });
}

Parameter Parameter::fromReference(UInt v)
{
	return Parameter(ParameterType::Reference, { v });
}

Parameter Parameter::fromBoolArray(const std::vector<bool>& v)
{
	std::vector<Variant> data;
	data.reserve(v.size());
	for (auto b : v)
		data.emplace_back(b);

	return Parameter(ParameterType::Bool, data);
}

Parameter Parameter::fromIntArray(const std::vector<Int>& v)
{
	std::vector<Variant> data;
	data.reserve(v.size());
	for (auto b : v)
		data.emplace_back(b);

	return Parameter(ParameterType::Int, data);
}

Parameter Parameter::fromUIntArray(const std::vector<uint64>& v)
{
	std::vector<Variant> data;
	data.reserve(v.size());
	for (auto b : v)
		data.emplace_back(b);

	return Parameter(ParameterType::UInt, data);
}

Parameter Parameter::fromNumberArray(const std::vector<Number>& v)
{
	std::vector<Variant> data;
	data.reserve(v.size());
	for (auto b : v)
		data.emplace_back(b);

	return Parameter(ParameterType::Number, data);
}

Parameter Parameter::fromStringArray(const std::vector<std::string>& v)
{
	std::vector<Variant> data;
	data.reserve(v.size());
	for (auto b : v)
		data.emplace_back(b);

	return Parameter(ParameterType::String, data);
}

} // namespace PR