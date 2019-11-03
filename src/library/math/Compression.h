#pragma once

#include "PR_Config.h"

namespace PR {
/* Implementation is based on:
*
* Zina H. Cigolle, Sam Donow, Daniel Evangelakos, Michael Mara, Morgan McGuire, and Quirin Meyer,
* Survey of Efficient Representations for Independent Unit Vectors,
* Journal of Computer Graphics Techniques (JCGT), vol. 3, no. 2, 1-30, 2014
*/

// Unsigned normalised floating point representation [0,1]
template <typename T>
inline T to_unorm(float f)
{
	return static_cast<T>(std::roundf(
		std::min<float>(std::max<float>(f, 0), 1) * std::numeric_limits<T>::max()));
}

template <typename T>
inline float from_unorm(T v)
{
	return static_cast<float>(v) / std::numeric_limits<T>::max();
}

typedef uint16 unorm16;
inline unorm16 to_unorm16(float f) { return to_unorm<unorm16>(f); }
inline float from_unorm16(unorm16 s) { return from_unorm<unorm16>(s); }

// Signed normalised floating point representation [-1,1]
template <typename T>
inline T to_snorm(float f)
{
	static_assert(std::is_signed<T>::value, "Require signed type");
	return static_cast<T>(std::roundf(
		std::min<float>(std::max<float>(f, -1), 1) * ((1 << std::numeric_limits<T>::digits) - 1)));
}

template <typename T>
inline float from_snorm(T v)
{
	static_assert(std::is_signed<T>::value, "Require signed type");
	return std::min<float>(std::max<float>(
							   static_cast<float>(v) / ((1 << std::numeric_limits<T>::digits) - 1),
							   -1),
						   1);
}

typedef int16 snorm16;
inline snorm16 to_snorm16(float f) { return to_snorm<snorm16>(f); }
inline float from_snorm16(snorm16 s) { return from_snorm<snorm16>(s); }

/* Octahedron Projection method
* MEYER, Q., SÜSSMUTH, J., SUSSNER, G., STAMMINGER, M., AND GREINER, G. 2010.
* On floating-point normal vectors.
* In Proceedings of the 21st Eurographics conference on Rendering,
* Eurographics Association, Aire-la-Ville, Switzerland, Switzerland, EGSR’10, 1405–1409.
* http://dx.doi.org/10.1111/j.1467-8659.2010.01737.x.
*/
inline void to_oct(float ix, float iy, float iz, float& ox, float& oy)
{
	// We assume [ix,iy,iz] is normalised!

	// Project
	const float a = std::abs(ix) + std::abs(iy) + std::abs(iz);
	ox			  = ix / a;
	oy			  = iy / a;

	// Reflect
	if (iz <= 0) {
		const float tx = (1 - std::abs(oy)) * ((ox >= 0) ? 1 : -1);
		oy			   = (1 - std::abs(ox)) * ((oy >= 0) ? 1 : -1);
		ox			   = tx;
	}
}

inline Vector2f to_oct(const Vector3f& v)
{
	float ox, oy;
	to_oct(v(0), v(1), v(2), ox, oy);
	return Vector2f(ox, oy);
}

inline void from_oct(float ix, float iy, float& ox, float& oy, float& oz)
{
	ox = ix;
	oy = iy;
	oz = 1 - std::abs(ix) - std::abs(iy);

	// Reflect
	if (oz < 0) {
		float tx = (1 - std::abs(oy)) * ((ox >= 0) ? 1 : -1);
		oy		 = (1 - std::abs(ox)) * ((oy >= 0) ? 1 : -1);
		ox		 = tx;
	}

	// Norm
	const float a = std::sqrt(ox * ox + oy * oy + oz * oz);
	ox /= a;
	oy /= a;
	oz /= a;
}

inline Vector3f from_oct(const Vector2f& d)
{
	float ox, oy, oz;
	from_oct(d(0), d(1), ox, oy, oz);
	return Vector3f(ox, oy, oz);
}

// Class for oct compression
template <typename T>
class PR_LIB_INLINE octNormal {
private:
	T mValue[2];

public:
	inline octNormal()
		: mValue{ 0, 0 } {};
	octNormal(const octNormal& other) = default;
	octNormal(octNormal&& other)	  = default;
	virtual ~octNormal()			  = default;

	inline octNormal(float px, float py, float pz)
	{
		float d0, d1;
		to_oct(px, py, pz, d0, d1);
		mValue[0] = to_snorm<T>(d0);
		mValue[1] = to_snorm<T>(d1);
	}

	inline explicit octNormal(const Vector3f& p)
		: octNormal(p(0), p(1), p(2))
	{
	}

	octNormal& operator=(const octNormal& other) = default;
	octNormal& operator=(octNormal&& other) = default;

	inline octNormal& operator=(const Vector3f& p)
	{
		const Vector2f d = to_oct(p);
		mValue[0]				= to_snorm<T>(d(0));
		mValue[1]				= to_snorm<T>(d(1));

		return *this;
	}

	// Access
	inline T operator[](int index) const
	{
		return mValue[index];
	}

	inline T& operator[](int index)
	{
		return mValue[index];
	}

	inline T operator()(int index) const
	{
		return mValue[index];
	}

	inline T& operator()(int index)
	{
		return mValue[index];
	}

	// Convert
	inline operator Vector3f() const
	{
		return toVector();
	}

	inline Vector3f toVector() const
	{
		return from_oct(Vector2f(
			from_snorm<T>(mValue[0]), from_snorm<T>(mValue[1])));
	}
};

// TODO: Implementation of octNormal24 would be good
typedef octNormal<int8> octNormal16;
typedef octNormal<int16> octNormal32;
} // namespace PR