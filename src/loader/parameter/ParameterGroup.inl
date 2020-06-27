// IWYU pragma: private, include "parameter/ParameterGroup.h"
namespace PR {
inline ParameterGroup::ParameterGroup()
{
}

// Named parameters
inline void ParameterGroup::addParameter(const std::string& name, const Parameter& param)
{
	PR_ASSERT(!name.empty(), "Parameter name has to be qualified!");
	PR_ASSERT(param.isValid(), "Parameter has to be valid!");
	mParameters[name] = param;
}

inline bool ParameterGroup::hasParameter(const std::string& name) const
{
	return mParameters.count(name) > 0;
}

inline Parameter ParameterGroup::getParameter(const std::string& name) const
{
	if (mParameters.count(name) > 0)
		return mParameters.at(name);
	else
		return Parameter();
}

inline const std::unordered_map<std::string, Parameter>& ParameterGroup::parameters() const { return mParameters; }
inline const std::vector<Parameter>& ParameterGroup::positionalParameters() const { return mPositionalParameters; }

inline bool ParameterGroup::getBool(const std::string& name, bool def) const { return getParameter(name).getBool(def); }
inline int64 ParameterGroup::getInt(const std::string& name, int64 def) const { return getParameter(name).getInt(def); }
inline int64 ParameterGroup::getExactInt(const std::string& name, int64 def) const { return getParameter(name).getExactInt(def); }
inline uint64 ParameterGroup::getUInt(const std::string& name, uint64 def) const { return getParameter(name).getUInt(def); }
inline uint64 ParameterGroup::getExactUInt(const std::string& name, uint64 def) const { return getParameter(name).getExactUInt(def); }
inline float ParameterGroup::getNumber(const std::string& name, float def) const { return getParameter(name).getNumber(def); }
inline float ParameterGroup::getExactNumber(const std::string& name, float def) const { return getParameter(name).getExactNumber(def); }
inline std::string ParameterGroup::getString(const std::string& name, const std::string& def) const { return getParameter(name).getString(def); }

inline bool ParameterGroup::getBool(const std::string& name, size_t ind, bool def) const { return getParameter(name).getBool(ind, def); }
inline int64 ParameterGroup::getInt(const std::string& name, size_t ind, int64 def) const { return getParameter(name).getInt(ind, def); }
inline int64 ParameterGroup::getExactInt(const std::string& name, size_t ind, int64 def) const { return getParameter(name).getExactInt(ind, def); }
inline uint64 ParameterGroup::getUInt(const std::string& name, size_t ind, uint64 def) const { return getParameter(name).getUInt(ind, def); }
inline uint64 ParameterGroup::getExactUInt(const std::string& name, size_t ind, uint64 def) const { return getParameter(name).getExactUInt(ind, def); }
inline float ParameterGroup::getNumber(const std::string& name, size_t ind, float def) const { return getParameter(name).getNumber(ind, def); }
inline float ParameterGroup::getExactNumber(const std::string& name, size_t ind, float def) const { return getParameter(name).getExactNumber(ind, def); }
inline std::string ParameterGroup::getString(const std::string& name, size_t ind, const std::string& def) const { return getParameter(name).getString(ind, def); }

inline std::vector<bool> ParameterGroup::getBoolArray(const std::string& name) const { return getParameter(name).getBoolArray(); }
inline std::vector<int64> ParameterGroup::getIntArray(const std::string& name) const { return getParameter(name).getIntArray(); }
inline std::vector<int64> ParameterGroup::getExactIntArray(const std::string& name) const { return getParameter(name).getExactIntArray(); }
inline std::vector<uint64> ParameterGroup::getUIntArray(const std::string& name) const { return getParameter(name).getUIntArray(); }
inline std::vector<uint64> ParameterGroup::getExactUIntArray(const std::string& name) const { return getParameter(name).getExactUIntArray(); }
inline std::vector<float> ParameterGroup::getNumberArray(const std::string& name) const { return getParameter(name).getNumberArray(); }
inline std::vector<float> ParameterGroup::getExactNumberArray(const std::string& name) const { return getParameter(name).getExactNumberArray(); }
inline std::vector<std::string> ParameterGroup::getStringArray(const std::string& name) const { return getParameter(name).getStringArray(); }

inline Vector2f ParameterGroup::getVector2f(const std::string& name, const Vector2f& def) const
{
	Parameter param = getParameter(name);
	if (param.arraySize() == 2) {
		return Vector2f(
			param.getNumber(0, def(0)),
			param.getNumber(1, def(1)));
	} else {
		return def;
	}
}

inline Vector3f ParameterGroup::getVector3f(const std::string& name, const Vector3f& def) const
{
	Parameter param = getParameter(name);
	if (param.arraySize() == 3) {
		return Vector3f(
			param.getNumber(0, def(0)),
			param.getNumber(1, def(1)),
			param.getNumber(2, def(2)));
	} else {
		return def;
	}
}

inline Eigen::Matrix3f ParameterGroup::getMatrix3f(const std::string& name, const Eigen::Matrix3f& def) const
{
	std::vector<float> arr = getNumberArray(name);
	if (arr.size() == 9)
		return Eigen::Map<Eigen::Matrix3f>(arr.data());
	else
		return def;
}

inline Eigen::Matrix4f ParameterGroup::getMatrix4f(const std::string& name, const Eigen::Matrix4f& def) const
{
	std::vector<float> arr = getNumberArray(name);
	if (arr.size() == 16)
		return Eigen::Map<Eigen::Matrix4f>(arr.data());
	else
		return def;
}

// Positional parameters
// Named parameters
inline void ParameterGroup::addParameter(const Parameter& param)
{
	PR_ASSERT(param.isValid(), "Parameter has to be valid!");
	mPositionalParameters.push_back(param);
}

inline bool ParameterGroup::hasParameter(size_t index) const
{
	return index < mPositionalParameters.size();
}

inline Parameter ParameterGroup::getParameter(size_t index) const
{
	if (index < mPositionalParameters.size())
		return mPositionalParameters[index];
	else
		return Parameter();
}

inline bool ParameterGroup::getBool(size_t index, bool def) const { return getParameter(index).getBool(def); }
inline int64 ParameterGroup::getInt(size_t index, int64 def) const { return getParameter(index).getInt(def); }
inline int64 ParameterGroup::getExactInt(size_t index, int64 def) const { return getParameter(index).getExactInt(def); }
inline uint64 ParameterGroup::getUInt(size_t index, uint64 def) const { return getParameter(index).getUInt(def); }
inline uint64 ParameterGroup::getExactUInt(size_t index, uint64 def) const { return getParameter(index).getExactUInt(def); }
inline float ParameterGroup::getNumber(size_t index, float def) const { return getParameter(index).getNumber(def); }
inline float ParameterGroup::getExactNumber(size_t index, float def) const { return getParameter(index).getExactNumber(def); }
inline std::string ParameterGroup::getString(size_t index, const std::string& def) const { return getParameter(index).getString(def); }

inline bool ParameterGroup::getBool(size_t index, size_t ind, bool def) const { return getParameter(index).getBool(ind, def); }
inline int64 ParameterGroup::getInt(size_t index, size_t ind, int64 def) const { return getParameter(index).getInt(ind, def); }
inline int64 ParameterGroup::getExactInt(size_t index, size_t ind, int64 def) const { return getParameter(index).getExactInt(ind, def); }
inline uint64 ParameterGroup::getUInt(size_t index, size_t ind, uint64 def) const { return getParameter(index).getUInt(ind, def); }
inline uint64 ParameterGroup::getExactUInt(size_t index, size_t ind, uint64 def) const { return getParameter(index).getExactUInt(ind, def); }
inline float ParameterGroup::getNumber(size_t index, size_t ind, float def) const { return getParameter(index).getNumber(ind, def); }
inline float ParameterGroup::getExactNumber(size_t index, size_t ind, float def) const { return getParameter(index).getExactNumber(ind, def); }
inline std::string ParameterGroup::getString(size_t index, size_t ind, const std::string& def) const { return getParameter(index).getString(ind, def); }

inline std::vector<bool> ParameterGroup::getBoolArray(size_t index) const { return getParameter(index).getBoolArray(); }
inline std::vector<int64> ParameterGroup::getIntArray(size_t index) const { return getParameter(index).getIntArray(); }
inline std::vector<int64> ParameterGroup::getExactIntArray(size_t index) const { return getParameter(index).getExactIntArray(); }
inline std::vector<uint64> ParameterGroup::getUIntArray(size_t index) const { return getParameter(index).getUIntArray(); }
inline std::vector<uint64> ParameterGroup::getExactUIntArray(size_t index) const { return getParameter(index).getExactUIntArray(); }
inline std::vector<float> ParameterGroup::getNumberArray(size_t index) const { return getParameter(index).getNumberArray(); }
inline std::vector<float> ParameterGroup::getExactNumberArray(size_t index) const { return getParameter(index).getExactNumberArray(); }
inline std::vector<std::string> ParameterGroup::getStringArray(size_t index) const { return getParameter(index).getStringArray(); }

inline Vector2f ParameterGroup::getVector2f(size_t index, const Vector2f& def) const
{
	Parameter param = getParameter(index);
	if (param.arraySize() == 2) {
		return Vector2f(
			param.getNumber(0, def(0)),
			param.getNumber(1, def(1)));
	} else {
		return def;
	}
}

inline Vector3f ParameterGroup::getVector3f(size_t index, const Vector3f& def) const
{
	Parameter param = getParameter(index);
	if (param.arraySize() == 3) {
		return Vector3f(
			param.getNumber(0, def(0)),
			param.getNumber(1, def(1)),
			param.getNumber(2, def(2)));
	} else {
		return def;
	}
}

inline Eigen::Matrix3f ParameterGroup::getMatrix3f(size_t index, const Eigen::Matrix3f& def) const
{
	std::vector<float> arr = getNumberArray(index);
	if (arr.size() == 9)
		return Eigen::Map<Eigen::Matrix3f>(arr.data());
	else
		return def;
}

inline Eigen::Matrix4f ParameterGroup::getMatrix4f(size_t index, const Eigen::Matrix4f& def) const
{
	std::vector<float> arr = getNumberArray(index);
	if (arr.size() == 16)
		return Eigen::Map<Eigen::Matrix4f>(arr.data());
	else
		return def;
}
} // namespace PR
