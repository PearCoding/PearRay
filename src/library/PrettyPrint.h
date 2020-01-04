#pragma once

#include "PR_Config.h"

namespace PR {
template <typename OtherDerived>
inline std::string matrix2String(const Eigen::DenseBase<OtherDerived>& m)
{
	static const Eigen::IOFormat sEigenFormatRowV(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", ", ", "", "", "[", "]");
	static const Eigen::IOFormat sEigenFormatColV(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", ", ", "{", "}", "", "");
	static const Eigen::IOFormat sEigenFormatMat(Eigen::StreamPrecision, 0, ", ", ", ", "{", "}", "[", "]");

	std::stringstream stream;
	if (m.rows() == 1 && m.cols() == 1)
		stream << m(0);
	else if (m.rows() == 1)
		stream << m.format(sEigenFormatColV);
	else if (m.cols() == 1)
		stream << m.format(sEigenFormatRowV);
	else
		stream << m.format(sEigenFormatMat);

	return stream.str();
}

inline std::string matrix2String(const Eigen::Quaternionf& m)
{
	return matrix2String(m.coeffs());
}

inline std::string memsize2String(size_t size)
{
	std::stringstream stream;
	if (size < 1024) {
		stream << size << "b";
		return stream.str();
	}

	size /= 1024;

	if (size < 1024) {
		stream << size << "kb";
		return stream.str();
	}

	size /= 1024;

	if (size < 1024) {
		stream << size << "Mb";
		return stream.str();
	}

	stream << (size / 1024) << "Gb";
	return stream.str();
}

} // namespace PR

#define PR_FMT_MAT(m) PR::matrix2String((m))
#define PR_FMT_MEM(s) PR::memsize2String((s))
