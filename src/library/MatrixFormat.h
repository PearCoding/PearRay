#pragma once

#include "PR_Config.h"

namespace PR {

template <int N>
inline std::string matrix2String(const Eigen::Matrix<float, N, 1>& m)
{
	static const Eigen::IOFormat sEigenFormatRowV(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", ", ", "", "", "[", "]");

	std::stringstream s;
	s << m.format(sEigenFormatRowV);
	return s.str();
}

template <int N>
inline std::string matrix2String(const Eigen::Matrix<float, 1, N>& m)
{
	static const Eigen::IOFormat sEigenFormatColV(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", ", ", "{", "}", "", "");

	std::stringstream s;
	s << m.format(sEigenFormatColV);
	return s.str();
}

template <int R, int C>
inline std::string matrix2String(const Eigen::Matrix<float, R, C>& m)
{
	static const Eigen::IOFormat sEigenFormatMat(Eigen::StreamPrecision, 0, ", ", ", ", "{", "}", "[", "]");

	std::stringstream s;
	s << m.format(sEigenFormatMat);
	return s.str();
}

inline std::string matrix2String(const Eigen::Quaternionf& m)
{
	return matrix2String(m.coeffs());
}

template <typename OtherDerived>
inline std::string matrix2String(const Eigen::MatrixBase<OtherDerived>& m)
{
	return matrix2String(m.eval());
}

} // namespace PR

#define PR_FMT_MAT(m) PR::matrix2String((m))
