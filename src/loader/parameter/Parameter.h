#pragma once

#include "PR_Config.h"
#include <vector>

namespace PR {
enum ParameterType : uint8 {
	PT_Invalid = 0,
	PT_Bool,
	PT_Int,
	PT_UInt,
	PT_Number,
	PT_String,
	PT_Reference // Uses uint
};

constexpr uint64 P_INVALID_REFERENCE = (uint64)-1;

class PR_LIB_LOADER Parameter final {
public:
	inline Parameter();

	Parameter(const Parameter&) = default;
	Parameter& operator=(const Parameter&) = default;

	Parameter(Parameter&&) = default;
	Parameter& operator=(Parameter&&) = default;

	static Parameter fromBool(bool v);
	static Parameter fromInt(int64 v);
	static Parameter fromUInt(uint64 v);
	static Parameter fromNumber(float v);
	static Parameter fromString(const std::string& v);
	static Parameter fromReference(uint64 id);

	static Parameter fromBoolArray(const std::vector<bool>& v);
	static Parameter fromIntArray(const std::vector<int64> v);
	static Parameter fromUIntArray(const std::vector<uint64> v);
	static Parameter fromNumberArray(const std::vector<float> v);
	static Parameter fromStringArray(const std::vector<std::string>& v);

	inline bool isValid() const;
	inline bool isReference() const;
	inline bool isArray() const;
	inline size_t arraySize() const;

	inline ParameterType type() const;

	inline bool getBool(bool def) const;
	inline int64 getInt(int64 def) const;
	inline int64 getExactInt(int64 def) const;
	inline uint64 getUInt(uint64 def) const;
	inline uint64 getExactUInt(uint64 def) const;
	inline float getNumber(float def) const;
	inline float getExactNumber(float def) const;
	inline std::string getString(const std::string& def) const;
	inline uint64 getReference(uint64 def = P_INVALID_REFERENCE) const;

	inline bool getBool(size_t ind, bool def) const;
	inline int64 getInt(size_t ind, int64 def) const;
	inline int64 getExactInt(size_t ind, int64 def) const;
	inline uint64 getUInt(size_t ind, uint64 def) const;
	inline uint64 getExactUInt(size_t ind, uint64 def) const;
	inline float getNumber(size_t ind, float def) const;
	inline float getExactNumber(size_t ind, float def) const;
	inline std::string getString(size_t ind, const std::string& def) const;

	inline std::vector<bool> getBoolArray() const;
	inline std::vector<int64> getIntArray() const;
	inline std::vector<int64> getExactIntArray() const;
	inline std::vector<uint64> getUIntArray() const;
	inline std::vector<uint64> getExactUIntArray() const;
	inline std::vector<float> getNumberArray() const;
	inline std::vector<float> getExactNumberArray() const;
	inline std::vector<std::string> getStringArray() const;

private:
	struct ParameterData {
		std::string String;
		union {
			bool Bool;
			int64 Int;
			uint64 UInt;
			float Number;
		};
	};
	inline Parameter(ParameterType t, const std::vector<ParameterData>& dt);

	ParameterType mType;
	std::vector<ParameterData> mData;
};

} // namespace PR

#include "Parameter.inl"