#pragma once

#include "PR_Config.h"

#include <variant>
#include <vector>

namespace PR {
enum class ParameterType : uint8 {
	Invalid = 0,
	Bool,
	Int,
	UInt,
	Number,
	String,
	Reference // Uses uint
};

constexpr uint64 P_INVALID_REFERENCE = (uint64)-1;

class ParameterGroup;
class PR_LIB_LOADER Parameter final {
public:
	using Int	 = int64;
	using UInt	 = uint64;
	using Number = float;

	inline Parameter();

	Parameter(const Parameter&) = default;
	Parameter& operator=(const Parameter&) = default;

	Parameter(Parameter&&) = default;
	Parameter& operator=(Parameter&&) = default;

	static Parameter fromBool(bool v);
	static Parameter fromInt(Int v);
	static Parameter fromUInt(UInt v);
	static Parameter fromNumber(Number v);
	static Parameter fromString(const std::string& v);
	static Parameter fromReference(UInt id);

	static Parameter fromBoolArray(const std::vector<bool>& v);
	static Parameter fromIntArray(const std::vector<Int>& v);
	static Parameter fromUIntArray(const std::vector<UInt>& v);
	static Parameter fromNumberArray(const std::vector<Number>& v);
	static Parameter fromStringArray(const std::vector<std::string>& v);

	inline bool isValid() const;
	inline bool isReference() const;
	inline bool isArray() const;
	inline size_t arraySize() const;

	inline ParameterType type() const;
	inline bool canBeNumber() const;

	inline bool getBool(bool def) const;
	inline Int getInt(Int def) const;
	inline Int getExactInt(Int def) const;
	inline UInt getUInt(UInt def) const;
	inline UInt getExactUInt(UInt def) const;
	inline Number getNumber(Number def) const;
	inline Number getExactNumber(Number def) const;
	inline std::string getString(const std::string& def) const;
	inline UInt getReference(UInt def = P_INVALID_REFERENCE) const;

	inline bool getBool(size_t ind, bool def) const;
	inline Int getInt(size_t ind, Int def) const;
	inline Int getExactInt(size_t ind, Int def) const;
	inline UInt getUInt(size_t ind, UInt def) const;
	inline UInt getExactUInt(size_t ind, UInt def) const;
	inline Number getNumber(size_t ind, Number def) const;
	inline Number getExactNumber(size_t ind, Number def) const;
	inline std::string getString(size_t ind, const std::string& def) const;

	inline std::vector<bool> getBoolArray() const;
	inline std::vector<Int> getIntArray() const;
	inline std::vector<Int> getExactIntArray() const;
	inline std::vector<UInt> getUIntArray() const;
	inline std::vector<UInt> getExactUIntArray() const;
	inline std::vector<Number> getNumberArray() const;
	inline std::vector<Number> getExactNumberArray() const;
	inline std::vector<std::string> getStringArray() const;

private:
	using Variant = std::variant<bool, Int, UInt, Number, std::string>;
	inline Parameter(ParameterType t, const std::vector<Variant>& dt);

	ParameterType mType;
	std::vector<Variant> mData;
};

} // namespace PR

#include "Parameter.inl"