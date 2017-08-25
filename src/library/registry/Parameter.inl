namespace PR {
inline ParameterAccessException::ParameterAccessException(ParameterType type) noexcept
	: std::exception()
	, mType(type)
{
}

inline char const* ParameterAccessException::what() const noexcept
{
	switch (mType) {
	case PT_None:
		return "Can not access parameter. Type is None.";
	case PT_Boolean:
		return "Can not access parameter. Type is not Boolean.";
	case PT_Integer:
		return "Can not access parameter. Type is not Integer.";
	case PT_Float:
		return "Can not access parameter. Type is not Float.";
	case PT_Vector2:
		return "Can not access parameter. Type is not Vector2.";
	case PT_Vector3:
		return "Can not access parameter. Type is not Vector3.";
	case PT_Vector4:
		return "Can not access parameter. Type is not Vector4.";
	case PT_Matrix3:
		return "Can not access parameter. Type is not Matrix3.";
	case PT_Matrix4:
		return "Can not access parameter. Type is not Matrix4.";
	case PT_Spectrum:
		return "Can not access parameter. Type is not Spectrum.";
	case PT_String:
		return "Can not access parameter. Type is not String.";
	default:
		return "-1";
	}
}

inline ParameterType ParameterAccessException::type() const
{
	return mType;
}

//////////////////////////////////////////////////////////////////////////////////
inline Parameter::Parameter()
	: mType(PT_None)
	, mBoolean(false)
{
}

inline Parameter::Parameter(bool b)
	: mType(PT_Boolean)
	, mBoolean(b)
{
}

inline Parameter::Parameter(int b)
	: mType(PT_Integer)
	, mInteger(b)
{
}

inline Parameter::Parameter(float b)
	: mType(PT_Float)
	, mFloat(b)
{
}

inline Parameter::Parameter(const Eigen::Vector2f& v)
	: mType(PT_Vector2)
	, mVector2(v)
{
}

inline Parameter::Parameter(const Eigen::Vector3f& v)
	: mType(PT_Vector3)
	, mVector3(v)
{
}

inline Parameter::Parameter(const Eigen::Vector4f& v)
	: mType(PT_Vector4)
	, mVector4(v)
{
}

inline Parameter::Parameter(const Eigen::Matrix3f& m)
	: mType(PT_Matrix3)
	, mMatrix3(m)
{
}

inline Parameter::Parameter(const Eigen::Matrix4f& m)
	: mType(PT_Matrix4)
	, mMatrix4(m)
{
}

inline Parameter::Parameter(const Spectrum& spec)
	: mType(PT_Spectrum)
	, mSpectrum(spec)
{
}

inline Parameter::Parameter(const std::string& str)
	: mType(PT_String)
	, mString(str)
{
}

inline ParameterType Parameter::type() const { return mType; }

#define _PR_PARAMETER_GETSET(type, rettype, name)      \
	inline rettype Parameter::get##name() const        \
	{                                                  \
		if (mType != PT_##name)                        \
			throw ParameterAccessException(PT_##name); \
		return m##name;                                \
	}                                                  \
	inline void Parameter::set##name(type b)           \
	{                                                  \
		mType   = PT_##name;                           \
		m##name = b;                                   \
	}

_PR_PARAMETER_GETSET(bool, bool, Boolean)
_PR_PARAMETER_GETSET(int, int, Integer)
_PR_PARAMETER_GETSET(float, float, Float)
_PR_PARAMETER_GETSET(const Eigen::Vector2f&, Eigen::Vector2f, Vector2)
_PR_PARAMETER_GETSET(const Eigen::Vector3f&, Eigen::Vector3f, Vector3)
_PR_PARAMETER_GETSET(const Eigen::Vector4f&, Eigen::Vector4f, Vector4)
_PR_PARAMETER_GETSET(const Eigen::Matrix3f&, Eigen::Matrix3f, Matrix3)
_PR_PARAMETER_GETSET(const Eigen::Matrix4f&, Eigen::Matrix4f, Matrix4)
_PR_PARAMETER_GETSET(const Spectrum&, const Spectrum&, Spectrum)
_PR_PARAMETER_GETSET(const std::string&, std::string, String)

#undef _PR_PARAMETER_GETSET
}