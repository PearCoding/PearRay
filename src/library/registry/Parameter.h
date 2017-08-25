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
	PT_Spectrum,
	PT_String
};

class PR_LIB_INLINE ParameterAccessException : public std::exception
{
public:
	inline ParameterAccessException(ParameterType type) noexcept;
	inline char const* what() const noexcept override;

	inline ParameterType type() const;

private:
	ParameterType mType;
};

class PR_LIB_INLINE Parameter {
public:
	inline Parameter();
	inline explicit Parameter(bool b);
	inline explicit Parameter(int b);
	inline explicit Parameter(float b);
	inline explicit Parameter(const Eigen::Vector2f& v);
	inline explicit Parameter(const Eigen::Vector3f& v);
	inline explicit Parameter(const Eigen::Vector4f& v);
	inline explicit Parameter(const Eigen::Matrix3f& m);
	inline explicit Parameter(const Eigen::Matrix4f& m);
	inline explicit Parameter(const Spectrum& spec);
	inline explicit Parameter(const std::string& str);

	inline ParameterType type() const;

	inline bool getBoolean() const;
	inline void setBoolean(bool b);

	inline int getInteger() const;
	inline void setInteger(int b);

	inline float getFloat() const;
	inline void setFloat(float b);

	inline Eigen::Vector2f getVector2() const;
	inline void setVector2(const Eigen::Vector2f& v);

	inline Eigen::Vector3f getVector3() const;
	inline void setVector3(const Eigen::Vector3f& v);

	inline Eigen::Vector4f getVector4() const;
	inline void setVector4(const Eigen::Vector4f& v);

	inline Eigen::Matrix3f getMatrix3() const;
	inline void setMatrix3(const Eigen::Matrix3f& v);

	inline Eigen::Matrix4f getMatrix4() const;
	inline void setMatrix4(const Eigen::Matrix4f& v);

	inline const Spectrum& getSpectrum() const;
	inline void setSpectrum(const Spectrum& s);

	inline std::string getString() const;
	inline void setString(const std::string& s);

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
	Spectrum mSpectrum;
	std::string mString;
};
}

#include "Parameter.inl"