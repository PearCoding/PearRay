// IWYU pragma: private, include "parameter/Parameter.h"
namespace PR {
inline Parameter::Parameter()
	: mFlags(0)
	, mType(PT_Invalid)
{
}

inline Parameter::Parameter(uint8 f, ParameterType t, const std::vector<ParameterData>& dt)
	: mFlags(f)
	, mType(t)
	, mData(dt)
{
}

inline bool Parameter::isValid() const { return !mData.empty(); }
inline bool Parameter::isArray() const { return mData.size() > 1 && mType != PT_Invalid; }

inline size_t Parameter::arraySize() const { return mData.size(); }

inline uint8 Parameter::flags() const { return mFlags; }
inline void Parameter::setFlags(uint8 f) { mFlags = f; }
inline void Parameter::assignFlags(uint8 f) { mFlags |= f; }

inline ParameterType Parameter::type() const { return mType; }

inline bool Parameter::getBool(bool def) const { return getBool(0, def); }
inline int64 Parameter::getInt(int64 def) const { return getInt(0, def); }
inline int64 Parameter::getExactInt(int64 def) const { return getExactInt(0, def); }
inline uint64 Parameter::getUInt(uint64 def) const { return getUInt(0, def); }
inline uint64 Parameter::getExactUInt(uint64 def) const { return getExactUInt(0, def); }
inline float Parameter::getNumber(float def) const { return getNumber(0, def); }
inline float Parameter::getExactNumber(float def) const { return getExactNumber(0, def); }
inline std::string Parameter::getString(const std::string& def) const { return getString(0, def); }

inline bool Parameter::getBool(size_t ind, bool def) const
{
	if (mType != PT_Bool || ind >= mData.size())
		return def;
	else
		return mData[ind].Bool;
}

inline int64 Parameter::getInt(size_t ind, int64 def) const
{
	if (ind >= mData.size())
		return def;
	else {
		switch (mType) {
		default:
			return def;
		case PT_Int:
			return mData[ind].Int;
		case PT_UInt:
			return static_cast<int64>(mData[ind].UInt);
		case PT_Number:
			return static_cast<int64>(mData[ind].Number);
		}
	}
}

inline int64 Parameter::getExactInt(size_t ind, int64 def) const
{
	if (mType != PT_Int || ind >= mData.size())
		return def;
	else
		return mData[ind].Int;
}

inline uint64 Parameter::getUInt(size_t ind, uint64 def) const
{
	if (ind >= mData.size())
		return def;
	else {
		switch (mType) {
		default:
			return def;
		case PT_Int:
			return static_cast<uint64>(mData[ind].Int);
		case PT_UInt:
			return mData[ind].UInt;
		case PT_Number:
			return static_cast<uint64>(mData[ind].Number);
		}
	}
}

inline uint64 Parameter::getExactUInt(size_t ind, uint64 def) const
{
	if (mType != PT_UInt || ind >= mData.size())
		return def;
	else {
		return mData[ind].UInt;
	}
}

inline float Parameter::getNumber(size_t ind, float def) const
{
	if (ind >= mData.size())
		return def;
	else {
		switch (mType) {
		default:
			return def;
		case PT_Int:
			return static_cast<float>(mData[ind].Int);
		case PT_UInt:
			return static_cast<float>(mData[ind].UInt);
		case PT_Number:
			return mData[ind].Number;
		}
	}
}

inline float Parameter::getExactNumber(size_t ind, float def) const
{
	if (mType != PT_Number || ind >= mData.size())
		return def;
	else {
		return mData[ind].Number;
	}
}

inline std::string Parameter::getString(size_t ind, const std::string& def) const
{
	if (mType != PT_String || ind >= mData.size())
		return def;
	else
		return mData[ind].String;
}

inline std::vector<bool> Parameter::getBoolArray() const
{
	if (mType != PT_Bool || mData.empty())
		return {};

	std::vector<bool> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = mData[i].Bool;
	return arr;
}

inline std::vector<int64> Parameter::getIntArray() const
{
	if (mData.empty())
		return {};

	std::vector<int64> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = getInt(i, 0);
	return arr;
}

inline std::vector<int64> Parameter::getExactIntArray() const
{
	if (mType != PT_Int || mData.empty())
		return {};

	std::vector<int64> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = mData[i].Int;
	return arr;
}

inline std::vector<uint64> Parameter::getUIntArray() const
{
	if (mData.empty())
		return {};

	std::vector<uint64> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = getUInt(i, 0);
	return arr;
}

inline std::vector<uint64> Parameter::getExactUIntArray() const
{
	if (mType != PT_UInt || mData.empty())
		return {};

	std::vector<uint64> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = mData[i].UInt;
	return arr;
}

inline std::vector<float> Parameter::getNumberArray() const
{
	if (mData.empty())
		return {};

	std::vector<float> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = getNumber(i, 0.0f);
	return arr;
}

inline std::vector<float> Parameter::getExactNumberArray() const
{
	if (mType != PT_Number || mData.empty())
		return {};

	std::vector<float> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = mData[i].Number;
	return arr;
}

inline std::vector<std::string> Parameter::getStringArray() const
{
	if (mType != PT_String || mData.empty())
		return {};

	std::vector<std::string> arr(arraySize());
	for (size_t i = 0; i < arraySize(); ++i)
		arr[i] = mData[i].String;
	return arr;
}
} // namespace PR