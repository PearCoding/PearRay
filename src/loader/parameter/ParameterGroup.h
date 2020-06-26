#pragma once

#include "Parameter.h"

#include <unordered_map>

namespace PR {
class PR_LIB_LOADER ParameterGroup final {
public:
	inline ParameterGroup();

	ParameterGroup(const ParameterGroup&) = default;
	ParameterGroup& operator=(const ParameterGroup&) = default;

	ParameterGroup(ParameterGroup&&) = default;
	ParameterGroup& operator=(ParameterGroup&&) = default;

	// Named parameters
	inline void addParameter(const std::string& name, const Parameter& param);
	inline bool hasParameter(const std::string& name) const;
	inline Parameter getParameter(const std::string& name) const;

	inline bool getBool(const std::string& name, bool def) const;
	inline int64 getInt(const std::string& name, int64 def) const;
	inline int64 getExactInt(const std::string& name, int64 def) const;
	inline uint64 getUInt(const std::string& name, uint64 def) const;
	inline uint64 getExactUInt(const std::string& name, uint64 def) const;
	inline float getNumber(const std::string& name, float def) const;
	inline float getExactNumber(const std::string& name, float def) const;
	inline std::string getString(const std::string& name, const std::string& def) const;

	inline bool getBool(const std::string& name, size_t ind, bool def) const;
	inline int64 getInt(const std::string& name, size_t ind, int64 def) const;
	inline int64 getExactInt(const std::string& name, size_t ind, int64 def) const;
	inline uint64 getUInt(const std::string& name, size_t ind, uint64 def) const;
	inline uint64 getExactUInt(const std::string& name, size_t ind, uint64 def) const;
	inline float getNumber(const std::string& name, size_t ind, float def) const;
	inline float getExactNumber(const std::string& name, size_t ind, float def) const;
	inline std::string getString(const std::string& name, size_t ind, const std::string& def) const;

	inline std::vector<bool> getBoolArray(const std::string& name) const;
	inline std::vector<int64> getIntArray(const std::string& name) const;
	inline std::vector<int64> getExactIntArray(const std::string& name) const;
	inline std::vector<uint64> getUIntArray(const std::string& name) const;
	inline std::vector<uint64> getExactUIntArray(const std::string& name) const;
	inline std::vector<float> getNumberArray(const std::string& name) const;
	inline std::vector<float> getExactNumberArray(const std::string& name) const;
	inline std::vector<std::string> getStringArray(const std::string& name) const;

	inline Vector2f getVector2f(const std::string& name, const Vector2f& def) const;
	inline Vector3f getVector3f(const std::string& name, const Vector3f& def) const;
	inline Eigen::Matrix3f getMatrix3f(const std::string& name, const Eigen::Matrix3f& def) const;
	inline Eigen::Matrix4f getMatrix4f(const std::string& name, const Eigen::Matrix4f& def) const;

	// Positional parameters
	inline void addParameter(const Parameter& param);
	inline bool hasParameter(size_t index) const;
	inline Parameter getParameter(size_t index) const;

	inline bool getBool(size_t index, bool def) const;
	inline int64 getInt(size_t index, int64 def) const;
	inline int64 getExactInt(size_t index, int64 def) const;
	inline uint64 getUInt(size_t index, uint64 def) const;
	inline uint64 getExactUInt(size_t index, uint64 def) const;
	inline float getNumber(size_t index, float def) const;
	inline float getExactNumber(size_t index, float def) const;
	inline std::string getString(size_t index, const std::string& def) const;

	inline bool getBool(size_t index, size_t ind, bool def) const;
	inline int64 getInt(size_t index, size_t ind, int64 def) const;
	inline int64 getExactInt(size_t index, size_t ind, int64 def) const;
	inline uint64 getUInt(size_t index, size_t ind, uint64 def) const;
	inline uint64 getExactUInt(size_t index, size_t ind, uint64 def) const;
	inline float getNumber(size_t index, size_t ind, float def) const;
	inline float getExactNumber(size_t index, size_t ind, float def) const;
	inline std::string getString(size_t index, size_t ind, const std::string& def) const;

	inline std::vector<bool> getBoolArray(size_t index) const;
	inline std::vector<int64> getIntArray(size_t index) const;
	inline std::vector<int64> getExactIntArray(size_t index) const;
	inline std::vector<uint64> getUIntArray(size_t index) const;
	inline std::vector<uint64> getExactUIntArray(size_t index) const;
	inline std::vector<float> getNumberArray(size_t index) const;
	inline std::vector<float> getExactNumberArray(size_t index) const;
	inline std::vector<std::string> getStringArray(size_t index) const;

	inline Vector2f getVector2f(size_t index, const Vector2f& def) const;
	inline Vector3f getVector3f(size_t index, const Vector3f& def) const;
	inline Eigen::Matrix3f getMatrix3f(size_t index, const Eigen::Matrix3f& def) const;
	inline Eigen::Matrix4f getMatrix4f(size_t index, const Eigen::Matrix4f& def) const;
private:
	std::unordered_map<std::string, Parameter> mParameters;
	std::vector<Parameter> mPositionalParameters;
};

} // namespace PR

#include "ParameterGroup.inl"