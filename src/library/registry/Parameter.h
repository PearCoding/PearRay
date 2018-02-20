#pragma once

#include "spectral/Spectrum.h"
#include <Eigen/Dense>

namespace PR {
enum ParameterType {
	PT_None = 0,
	PT_Boolean,
	PT_Integer,
	PT_Float,
	PT_Vector2,
	PT_Vector3,
	PT_Vector4,
	PT_Matrix3,
	PT_Matrix4,
	//PT_Spectrum,
	PT_String,

	PT_FrameBuffer3D,
	PT_FrameBuffer1D,
	PT_FrameBufferCounter,
	PT_FrameBufferSpectrum,

	PT_ScalarShaderOutput,
	PT_SpectrumShaderOutput,
	PT_VectorShaderOutput,
};

class FrameBuffer3D;
class FrameBuffer1D;
class FrameBufferCounter;
class FrameBufferSpectrum;

class ScalarShaderOutput;
class SpectrumShaderOutput;
class VectorShaderOutput;

class PR_LIB_INLINE ParameterAccessException : public std::exception {
public:
	inline explicit ParameterAccessException(ParameterType type) noexcept;
	inline char const* what() const noexcept override;

	inline ParameterType type() const;

private:
	ParameterType mType;
};

#define _PR_STD_PARAMETER_TYPE(type, name)    \
	inline explicit Parameter(const type& v); \
	inline type get##name() const;            \
	inline void set##name(const type& v)

#define _PR_STD_PARAMETER_TYPE_PTR(type, name)                 \
	inline explicit Parameter(const std::shared_ptr<type>& v); \
	inline std::shared_ptr<type> get##name() const;     \
	inline void set##name(const std::shared_ptr<type>& v)

class PR_LIB_INLINE Parameter {
public:
	inline Parameter();
	inline explicit Parameter(bool b);
	inline explicit Parameter(int b);
	inline explicit Parameter(float b);
	inline explicit Parameter(const Spectrum& spec);

	inline ParameterType type() const;
	inline std::string typeString() const;

	inline bool getBoolean() const;
	inline void setBoolean(bool b);

	inline int getInteger() const;
	inline void setInteger(int b);

	inline float getFloat() const;
	inline void setFloat(float b);

	/*inline const Spectrum& getSpectrum() const;
	inline void setSpectrum(const Spectrum& s);*/

	// Generic
	_PR_STD_PARAMETER_TYPE(Eigen::Vector2f, Vector2);
	_PR_STD_PARAMETER_TYPE(Eigen::Vector3f, Vector3);
	_PR_STD_PARAMETER_TYPE(Eigen::Vector4f, Vector4);
	_PR_STD_PARAMETER_TYPE(Eigen::Matrix3f, Matrix3);
	_PR_STD_PARAMETER_TYPE(Eigen::Matrix4f, Matrix4);
	_PR_STD_PARAMETER_TYPE(std::string, String);

	_PR_STD_PARAMETER_TYPE_PTR(FrameBuffer3D, FrameBuffer3D);
	_PR_STD_PARAMETER_TYPE_PTR(FrameBuffer1D, FrameBuffer1D);
	_PR_STD_PARAMETER_TYPE_PTR(FrameBufferCounter, FrameBufferCounter);
	_PR_STD_PARAMETER_TYPE_PTR(FrameBufferSpectrum, FrameBufferSpectrum);

	_PR_STD_PARAMETER_TYPE_PTR(ScalarShaderOutput, ScalarShaderOutput);
	_PR_STD_PARAMETER_TYPE_PTR(SpectrumShaderOutput, SpectrumShaderOutput);
	_PR_STD_PARAMETER_TYPE_PTR(VectorShaderOutput, VectorShaderOutput);

private:
	ParameterType mType;

	union {
		bool mBoolean;
		int mInteger;
		float mFloat;
	};

	Eigen::Matrix<float, 2, 1, Eigen::DontAlign> mVector2;
	Eigen::Matrix<float, 3, 1, Eigen::DontAlign> mVector3;
	Eigen::Matrix<float, 4, 1, Eigen::DontAlign> mVector4;
	Eigen::Matrix<float, 3, 3, Eigen::DontAlign> mMatrix3;
	Eigen::Matrix<float, 4, 4, Eigen::DontAlign> mMatrix4;
	//Spectrum mSpectrum;
	std::string mString;

	std::shared_ptr<FrameBuffer3D> mFrameBuffer3D;
	std::shared_ptr<FrameBuffer1D> mFrameBuffer1D;
	std::shared_ptr<FrameBufferCounter> mFrameBufferCounter;
	std::shared_ptr<FrameBufferSpectrum> mFrameBufferSpectrum;

	std::shared_ptr<ScalarShaderOutput> mScalarShaderOutput;
	std::shared_ptr<SpectrumShaderOutput> mSpectrumShaderOutput;
	std::shared_ptr<VectorShaderOutput> mVectorShaderOutput;
};
} // namespace PR

#undef _PR_STD_PARAMETER_TYPE
#undef _PR_STD_PARAMETER_TYPE_PTR

#include "Parameter.inl"
