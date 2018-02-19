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
	/*case PT_Spectrum:
		return "Can not access parameter. Type is not Spectrum.";*/
	case PT_String:
		return "Can not access parameter. Type is not String.";

	case PT_FrameBuffer3D:
		return "Can not access parameter. Type is not FrameBuffer3D.";
	case PT_FrameBuffer1D:
		return "Can not access parameter. Type is not FrameBuffer1D.";
	case PT_FrameBufferCounter:
		return "Can not access parameter. Type is not FrameBufferCounter.";
	case PT_FrameBufferSpectrum:
		return "Can not access parameter. Type is not FrameBufferSpectrum.";

	case PT_ScalarShaderOutput:
		return "Can not access parameter. Type is not ScalarShaderOutput.";
	case PT_SpectrumShaderOutput:
		return "Can not access parameter. Type is not SpectrumShaderOutput.";
	case PT_VectorShaderOutput:
		return "Can not access parameter. Type is not VectorShaderOutput.";
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

inline ParameterType Parameter::type() const { return mType; }

inline std::string Parameter::typeString() const
{
	switch (mType) {
	case PT_None:
		return "None";
	case PT_Boolean:
		return "Boolean";
	case PT_Integer:
		return "Integer";
	case PT_Float:
		return "Float";
	case PT_Vector2:
		return "Vector2";
	case PT_Vector3:
		return "Vector3";
	case PT_Vector4:
		return "Vector4";
	case PT_Matrix3:
		return "Matrix3";
	case PT_Matrix4:
		return "Matrix4";
	/*case PT_Spectrum:
		return "Spectrum";*/
	case PT_String:
		return "String";

	case PT_FrameBuffer3D:
		return "FrameBuffer3D";
	case PT_FrameBuffer1D:
		return "FrameBuffer1D";
	case PT_FrameBufferCounter:
		return "FrameBufferCounter";
	case PT_FrameBufferSpectrum:
		return "FrameBufferSpectrum";

	case PT_ScalarShaderOutput:
		return "ScalarShaderOutput";
	case PT_SpectrumShaderOutput:
		return "SpectrumShaderOutput";
	case PT_VectorShaderOutput:
		return "VectorShaderOutput";
	default:
		return "Unknown";
	}
}

#define _PR_PARAMETER_GETSET(type, rettype, name)      \
	inline Parameter::Parameter(type v)                \
		: mType(PT_##name)                             \
		, m##name(v)                                   \
	{                                                  \
	}                                                  \
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
//_PR_PARAMETER_GETSET(const Spectrum&, const Spectrum&, Spectrum)
_PR_PARAMETER_GETSET(const std::string&, std::string, String)

_PR_PARAMETER_GETSET(const std::shared_ptr<FrameBuffer3D>&, std::shared_ptr<FrameBuffer3D>, FrameBuffer3D)
_PR_PARAMETER_GETSET(const std::shared_ptr<FrameBuffer1D>&, std::shared_ptr<FrameBuffer1D>, FrameBuffer1D)
_PR_PARAMETER_GETSET(const std::shared_ptr<FrameBufferCounter>&, std::shared_ptr<FrameBufferCounter>, FrameBufferCounter)
_PR_PARAMETER_GETSET(const std::shared_ptr<FrameBufferSpectrum>&, std::shared_ptr<FrameBufferSpectrum>, FrameBufferSpectrum)

_PR_PARAMETER_GETSET(const std::shared_ptr<ScalarShaderOutput>&, std::shared_ptr<ScalarShaderOutput>, ScalarShaderOutput)
_PR_PARAMETER_GETSET(const std::shared_ptr<SpectrumShaderOutput>&, std::shared_ptr<SpectrumShaderOutput>, SpectrumShaderOutput)
_PR_PARAMETER_GETSET(const std::shared_ptr<VectorShaderOutput>&, std::shared_ptr<VectorShaderOutput>, VectorShaderOutput)

#undef _PR_PARAMETER_GETSET
}
