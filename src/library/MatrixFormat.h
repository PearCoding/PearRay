#pragma once

#include "PR_Config.h"

namespace PR {
template <int _Rows, int _Options, int _MaxRows, int _MaxCols>
inline std::string matrix2String(const Eigen::Matrix<float, _Rows, 1, _Options, _MaxRows, _MaxCols>& m)
{
	static const Eigen::IOFormat sEigenFormatRowV(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", ", ", "", "", "[", "]");

	std::stringstream s;
	s << m.format(sEigenFormatRowV);
	return s.str();
}

template <int _Cols, int _Options, int _MaxRows, int _MaxCols>
inline std::string matrix2String(const Eigen::Matrix<float, 1, _Cols, _Options, _MaxRows, _MaxCols>& m)
{
	static const Eigen::IOFormat sEigenFormatColV(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", ", ", "{", "}", "", "");

	std::stringstream s;
	s << m.format(sEigenFormatColV);
	return s.str();
}

template <int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
inline std::string matrix2String(const Eigen::Matrix<float, _Rows, _Cols, _Options, _MaxRows, _MaxCols>& m)
{
	static const Eigen::IOFormat sEigenFormatMat(Eigen::StreamPrecision, 0, ", ", ", ", "{", "}", "[", "]");

	std::stringstream s;
	s << m.format(sEigenFormatMat);
	return s.str();
}

template <typename OtherDerived>
inline std::string matrix2String(const Eigen::MatrixBase<OtherDerived>& m)
{
	return matrix2String(m.eval());
}

inline std::string matrix2String(const Eigen::Quaternionf& m)
{
	return matrix2String(m.coeffs());
}

} // namespace PR

#define PR_FMT_MAT(m) PR::matrix2String((m))
