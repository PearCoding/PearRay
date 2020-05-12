// IWYU pragma: private, include "parameter/ParameterGroup.h"
namespace PR {
inline ParameterGroup::ParameterGroup()
{
}

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
} // namespace PR
