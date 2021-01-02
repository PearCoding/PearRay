// IWYU pragma: private, include "parameter/Parameter.h"
namespace PR {
inline Parameter::Parameter()
	: mType(ParameterType::Invalid)
{
}

inline Parameter::Parameter(ParameterType t, const std::vector<Variant>& dt)
	: mType(t)
	, mData(dt)
{
}

inline bool Parameter::isValid() const { return !mData.empty() && mType != ParameterType::Invalid; }
inline bool Parameter::isReference() const { return mData.size() == 1 && mType == ParameterType::Reference; }
inline bool Parameter::isArray() const { return mData.size() > 1 && mType != ParameterType::Invalid; }

inline size_t Parameter::arraySize() const { return mData.size(); }

inline ParameterType Parameter::type() const { return mType; }
inline bool Parameter::canBeNumber() const { return mType == ParameterType::Int || mType == ParameterType::UInt || mType == ParameterType::Number; }

inline bool Parameter::getBool(bool def) const { return getBool(0, def); }
inline Parameter::Int Parameter::getInt(Int def) const { return getInt(0, def); }
inline Parameter::Int Parameter::getExactInt(Int def) const { return getExactInt(0, def); }
inline Parameter::UInt Parameter::getUInt(UInt def) const { return getUInt(0, def); }
inline Parameter::UInt Parameter::getExactUInt(UInt def) const { return getExactUInt(0, def); }
inline Parameter::Number Parameter::getNumber(Number def) const { return getNumber(0, def); }
inline Parameter::Number Parameter::getExactNumber(Number def) const { return getExactNumber(0, def); }
inline std::string Parameter::getString(const std::string& def) const { return getString(0, def); }
inline Parameter::UInt Parameter::getReference(UInt def) const
{
	if (isReference())
		return std::get<UInt>(mData[0]);
	else
		return def;
}

inline bool Parameter::getBool(size_t ind, bool def) const
{
	if (mType != ParameterType::Bool || ind >= mData.size())
		return def;
	else
		return std::get<bool>(mData[ind]);
}

inline Parameter::Int Parameter::getInt(size_t ind, Int def) const
{
	if (ind >= mData.size()) {
		return def;
	} else {
		switch (mType) {
		default:
			return def;
		case ParameterType::Int:
			return std::get<Int>(mData[ind]);
		case ParameterType::UInt:
			return static_cast<Int>(std::get<UInt>(mData[ind]));
		case ParameterType::Number:
			return static_cast<Int>(std::get<Number>(mData[ind]));
		}
	}
}

inline Parameter::Int Parameter::getExactInt(size_t ind, Int def) const
{
	if (mType != ParameterType::Int || ind >= mData.size())
		return def;
	else
		return std::get<Int>(mData[ind]);
}

inline Parameter::UInt Parameter::getUInt(size_t ind, UInt def) const
{
	if (ind >= mData.size()) {
		return def;
	} else {
		switch (mType) {
		default:
			return def;
		case ParameterType::Int:
			return std::get<Int>(mData[ind]) >= 0 ? static_cast<UInt>(std::get<Int>(mData[ind])) : def;
		case ParameterType::UInt:
			return std::get<UInt>(mData[ind]);
		case ParameterType::Number:
			return std::get<Number>(mData[ind]) >= 0 ? static_cast<UInt>(std::get<Number>(mData[ind])) : def;
		}
	}
}

inline Parameter::UInt Parameter::getExactUInt(size_t ind, UInt def) const
{
	if (mType != ParameterType::UInt || ind >= mData.size())
		return def;
	else
		return std::get<UInt>(mData[ind]);
}

inline Parameter::Number Parameter::getNumber(size_t ind, Number def) const
{
	if (ind >= mData.size()) {
		return def;
	} else {
		switch (mType) {
		default:
			return def;
		case ParameterType::Int:
			return static_cast<Number>(std::get<Int>(mData[ind]));
		case ParameterType::UInt:
			return static_cast<Number>(std::get<UInt>(mData[ind]));
		case ParameterType::Number:
			return std::get<Number>(mData[ind]);
		}
	}
}

inline Parameter::Number Parameter::getExactNumber(size_t ind, Number def) const
{
	if (mType != ParameterType::Number || ind >= mData.size())
		return def;
	else
		return std::get<Number>(mData[ind]);
}

inline std::string Parameter::getString(size_t ind, const std::string& def) const
{
	if (mType != ParameterType::String || ind >= mData.size())
		return def;
	else
		return std::get<std::string>(mData[ind]);
}

inline std::vector<bool> Parameter::getBoolArray() const
{
	if (mType != ParameterType::Bool || mData.empty())
		return {};

	std::vector<bool> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = std::get<bool>(mData[i]);
	return arr;
}

inline std::vector<Parameter::Int> Parameter::getIntArray() const
{
	if (mData.empty())
		return {};

	std::vector<Int> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = getInt(i, 0);
	return arr;
}

inline std::vector<Parameter::Int> Parameter::getExactIntArray() const
{
	if (mType != ParameterType::Int || mData.empty())
		return {};

	std::vector<Int> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = std::get<Int>(mData[i]);
	return arr;
}

inline std::vector<Parameter::UInt> Parameter::getUIntArray() const
{
	if (mData.empty())
		return {};

	std::vector<UInt> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = getUInt(i, 0);
	return arr;
}

inline std::vector<Parameter::UInt> Parameter::getExactUIntArray() const
{
	if (mType != ParameterType::UInt || mData.empty())
		return {};

	std::vector<UInt> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = std::get<UInt>(mData[i]);
	return arr;
}

inline std::vector<Parameter::Number> Parameter::getNumberArray() const
{
	if (mData.empty())
		return {};

	std::vector<Number> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = getNumber(i, 0.0f);
	return arr;
}

inline std::vector<Parameter::Number> Parameter::getExactNumberArray() const
{
	if (mType != ParameterType::Number || mData.empty())
		return {};

	std::vector<Number> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = std::get<Number>(mData[i]);
	return arr;
}

inline std::vector<std::string> Parameter::getStringArray() const
{
	if (mType != ParameterType::String || mData.empty())
		return {};

	std::vector<std::string> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = std::get<std::string>(mData[i]);
	return arr;
}
} // namespace PR
