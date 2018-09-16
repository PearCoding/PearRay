#pragma once

#include "math/SIMD.h"

namespace PR {
class PR_LIB_INLINE Tangent {
	PR_CLASS_NON_CONSTRUCTABLE(Tangent);

public:
	// Align v on N
	static inline Eigen::Vector3f align(const Eigen::Vector3f& N, const Eigen::Vector3f& V)
	{
		return align(N, V, Eigen::Vector3f(0, 0, 1));
	}

	static inline Eigen::Vector3f align(const Eigen::Vector3f& N, const Eigen::Vector3f& V, const Eigen::Vector3f& axis)
	{
		const float dot = N.dot(axis);
		if (dot + 1 < PR_EPSILON)
			return -V;
		else if (dot < 1)
			return Eigen::Quaternionf::FromTwoVectors(axis, N) * V;

		return V;
	}

	// N Orientation Z+
	static inline void frame(const Eigen::Vector3f& N, Eigen::Vector3f& T, Eigen::Vector3f& B)
	{
		Eigen::Vector3f t = std::abs(N(0)) > 0.99f ? Eigen::Vector3f(0, 1, 0) : Eigen::Vector3f(1, 0, 0);
		T				  = N.cross(t).normalized();
		B				  = N.cross(T).normalized();
	}

	static inline Eigen::Vector3f _align(const Eigen::Vector3f& N, const Eigen::Vector3f& V)
	{
		Eigen::Vector3f X, Y;
		frame(N, X, Y);
		return align(N, X, Y, V);
	}

	static inline Eigen::Vector3f align(const Eigen::Vector3f& N,
										const Eigen::Vector3f& Nx, const Eigen::Vector3f& Ny, const Eigen::Vector3f& V)
	{
		return N * V(2) + Ny * V(1) + Nx * V(0);
	}

	// SIMD
	static inline void frameV(const vfloat& N1, const vfloat& N2, const vfloat& N3,
							  vfloat& Nx1, vfloat& Nx2, vfloat& Nx3,
							  vfloat& Ny1, vfloat& Ny2, vfloat& Ny3)
	{
		using namespace simdpp;

		bfloat m  = abs(N1) > 0.99f;
		vfloat zero = make_float(0);
		vfloat one = make_float(1);

		vfloat t1 = blend(zero, one, m);
		vfloat t2 = blend(one, zero, m);
		vfloat t3 = zero;

		crossV(N1, N2, N3, t1, t2, t3, Nx1, Nx2, Nx3);
		normalizeV(Nx1, Nx2, Nx3);
		crossV(N1, N2, N3, Nx1, Nx2, Nx3, Ny1, Ny2, Ny3);
		normalizeV(Ny1, Ny2, Ny3);
	}
};
} // namespace PR
