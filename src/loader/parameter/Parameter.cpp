#include "Parameter.h"

namespace PR {

 Parameter Parameter::fromBool(bool v)
{
	ParameterData data;
	data.Bool = v;
	return Parameter(ParameterType::Bool, { data });
}

 Parameter Parameter::fromInt(int64 v)
{
	ParameterData data;
	data.Int = v;
	return Parameter(ParameterType::Int, { data });
}

 Parameter Parameter::fromUInt(uint64 v)
{
	ParameterData data;
	data.UInt = v;
	return Parameter(ParameterType::UInt, { data });
}

 Parameter Parameter::fromNumber(float v)
{
	ParameterData data;
	data.Number = v;
	return Parameter(ParameterType::Number, { data });
}

 Parameter Parameter::fromString(const std::string& v)
{
	ParameterData data;
	data.String = v;
	return Parameter(ParameterType::String, { data });
}

 Parameter Parameter::fromReference(uint64 v)
{
	ParameterData data;
	data.UInt = v;
	return Parameter(ParameterType::Reference, { data });
}

 Parameter Parameter::fromBoolArray(const std::vector<bool>& v)
{
	std::vector<ParameterData> data;
	data.reserve(v.size());
	for (auto b : v) {
		ParameterData p;
		p.Bool = b;
		data.emplace_back(p);
	}
	return Parameter(ParameterType::Bool, data);
}

 Parameter Parameter::fromIntArray(const std::vector<int64> v)
{
	std::vector<ParameterData> data;
	data.reserve(v.size());
	for (auto b : v) {
		ParameterData p;
		p.Int = b;
		data.emplace_back(p);
	}
	return Parameter(ParameterType::Int, data);
}

 Parameter Parameter::fromUIntArray(const std::vector<uint64> v)
{
	std::vector<ParameterData> data;
	data.reserve(v.size());
	for (auto b : v) {
		ParameterData p;
		p.UInt = b;
		data.emplace_back(p);
	}
	return Parameter(ParameterType::UInt, data);
}

 Parameter Parameter::fromNumberArray(const std::vector<float> v)
{
	std::vector<ParameterData> data;
	data.reserve(v.size());
	for (auto b : v) {
		ParameterData p;
		p.Number = b;
		data.emplace_back(p);
	}
	return Parameter(ParameterType::Number, data);
}

 Parameter Parameter::fromStringArray(const std::vector<std::string>& v)
{
	std::vector<ParameterData> data;
	data.reserve(v.size());
	for (auto b : v) {
		ParameterData p;
		p.String = b;
		data.emplace_back(p);
	}
	return Parameter(ParameterType::String, data);
}

} // namespace PR