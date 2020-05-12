#pragma once

#include "PR_Config.h"

namespace PR {
namespace Tangent {

// Functions applying to the TBN matrix
template <typename T>
inline Vector3t<T> fromTangentSpace(const Vector3t<T>& N,
									const Vector3t<T>& Nx, const Vector3t<T>& Ny,
									const Vector3t<T>& V)
{
	return N * V(2) + Ny * V(1) + Nx * V(0);
}

template <typename T>
inline Vector3t<T> toTangentSpace(const Vector3t<T>& N,
								  const Vector3t<T>& Nx, const Vector3t<T>& Ny,
								  const Vector3t<T>& V)
{
	return Vector3t<T>(Nx(2), Ny(2), N(2)) * V(2) + Vector3t<T>(Nx(1), Ny(1), N(1)) * V(1) + Vector3t<T>(Nx(0), Ny(0), N(0)) * V(0);
}

// N Orientation Z+
/* Frisvad, J. R. (2012). Building an Orthonormal Basis from a 3D Unit Vector Without Normalization.
 * Journal ofGraphics Tools, 16(3), 151–159.
 * https://doi.org/10.1080/2165347X.2012.689606
 * 
 * Nelson Max, Improved accuracy when building an orthonormal basis,
 * Journal of Computer Graphics Techniques (JCGT), vol. 6, no. 1, 9–16, 2017
 * http://jcgt.org/published/0006/01/02/
 */
constexpr float PR_FRAME_FRISVAD_EPS = 0.9999805689f;
inline void frame_frisvad(const Vector3f& N, Vector3f& Nx, Vector3f& Ny)
{
	if (N(2) < -PR_FRAME_FRISVAD_EPS) {
		Nx = Vector3f(0, -1, 0);
		Ny = Vector3f(-1, 0, 0);
	} else {
		const float a = 1.0f / (1.0f + N(2));
		const float b = -N(0) * N(1) * a;
		Nx			  = Vector3f(1.0f - N(0) * N(0) * a, b, -N(0));
		Ny			  = Vector3f(b, 1.0f - N(1) * N(1) * a, -N(1));
	}
}

inline void frame_frisvad(const Vector3fv& N, Vector3fv& Nx, Vector3fv& Ny)
{
	const vfloat N_ONE = vfloat(-1);
	const vfloat ZERO  = vfloat(0);

	bfloat mask = N(2) < vfloat(-PR_FRAME_FRISVAD_EPS);

	const vfloat a = 1.0f / (1.0f + N(2));
	const vfloat b = -N(0) * N(1) * a;

	Nx = Vector3fv(
		blend(ZERO, 1.0f - N(0) * N(0) * a, mask),
		blend(N_ONE, b, mask),
		blend(ZERO, -N(0), mask));
	Ny = Vector3fv(
		blend(N_ONE, b, mask),
		blend(ZERO, 1.0f - N(1) * N(1) * a, mask),
		blend(ZERO, -N(1), mask));
}

/* Tom Duff, James Burgess, Per Christensen, Christophe Hery, Andrew Kensler, Max Liani, and Ryusuke Villemin,
 * Building an Orthonormal Basis, Revisited, Journal of Computer Graphics Techniques (JCGT), vol. 6, no. 1, 1-8, 2017
 * http://jcgt.org/published/0006/01/01/
 */
inline void frame_duff(const Vector3f& N, Vector3f& Nx, Vector3f& Ny)
{
	const float sign = copysignf(1.0f, N(2));
	const float a	 = -1.0f / (sign + N(2));
	const float b	 = N(0) * N(1) * a;
	Nx				 = Vector3f(1.0f + sign * N(0) * N(0) * a, sign * b, -sign * N(0));
	Ny				 = Vector3f(b, sign + N(1) * N(1) * a, -N(1));
}

inline void frame_duff(const Vector3fv& N, Vector3fv& Nx, Vector3fv& Ny)
{
	const vfloat sign = blend(vfloat(1), vfloat(-1), N(2) >= vfloat(0));
	const vfloat a	  = -1.0f / (sign + N(2));
	const vfloat b	  = N(0) * N(1) * a;
	Nx				  = Vector3fv(1.0f + sign * N(0) * N(0) * a, sign * b, -sign * N(0));
	Ny				  = Vector3fv(b, sign + N(1) * N(1) * a, -N(1));
}

template <typename T>
inline void frame(const Vector3t<T>& N, Vector3t<T>& Nx, Vector3t<T>& Ny)
{
#ifdef PR_USE_ORTHOGONAL_FRAME_FRISVAD
	return frame_frisvad(N, Nx, Ny);
#else
	return frame_duff(N, Nx, Ny);
#endif
}

template <typename T>
inline void invert_frame(Vector3t<T>& N, Vector3t<T>& Nx, Vector3t<T>& Ny)
{
	N = -N;

	Vector3t<T> t = Nx;
	Nx			  = -Ny;
	Ny			  = -t;
}

// Align v on N
template <typename T>
inline Vector3t<T> align(const Vector3t<T>& N, const Vector3t<T>& V)
{
	Vector3t<T> nx, ny;
	frame(N, nx, ny);
	return fromTangentSpace(N, nx, ny, V);
}

} // namespace Tangent
} // namespace PR
