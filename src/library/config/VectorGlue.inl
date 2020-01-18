// IWYU pragma: private, include "PR_Config.h"
namespace PR {

/* Regardless of optimal size, we have to enforce same components for all vectorized types*/
constexpr size_t PR_SIMD_BANDWIDTH = SIMDPP_FAST_FLOAT32_SIZE;

/* Overloaded fill functions used in the vector base */
inline auto fill_vector(float f) { return simdpp::make_float(f); }
inline auto fill_vector(int32 f) { return simdpp::make_int(f); }
inline auto fill_vector(uint32 f) { return simdpp::make_uint(f); }

/* Extends simdpp by default fill constructors and cwise assignment ops */
template <unsigned N, typename T, template <unsigned, class> class V>
class VectorBase : public V<N, void> {
public:
	using Base = V<N, void>;
	using Base::Base;
	using Base::operator=;

	/*template <class E>
	inline VectorBase(const V<N, E>& b)
		: Base(b)
	{
	}

	template <class E>
	inline VectorBase& operator=(const V<N, E>& a)
	{
		*this = VectorBase<N, T, V>(a);
		return *this;
	}*/

	inline VectorBase(const Base& b)
		: Base(b)
	{
	}

	inline VectorBase& operator=(const Base& a)
	{
		*this = VectorBase<N, T, V>(a);
		return *this;
	}

	inline VectorBase()
		: VectorBase(T(0))
	{
	}

	inline explicit VectorBase(T f)
		: Base(fill_vector(f))
	{
	}

	template <class E>
	inline VectorBase& operator+=(const V<N, E>& a)
	{
		*this = *this + a.eval();
		return *this;
	}

	template <class E>
	inline VectorBase& operator-=(const V<N, E>& a)
	{
		*this = *this - a.eval();
		return *this;
	}

	template <class E>
	inline VectorBase& operator*=(const V<N, E>& a)
	{
		*this = *this * a.eval();
		return *this;
	}

	template <class E>
	inline VectorBase& operator/=(const V<N, E>& a)
	{
		*this = *this / a.eval();
		return *this;
	}

	inline VectorBase operator-() const
	{
		return VectorBase<N, T, V>(simdpp::neg(*this));
	}
};

/* Three type base classes */
template <unsigned N>
using vnfloat = VectorBase<N, float, simdpp::float32>;
template <unsigned N>
using vnint32 = VectorBase<N, int32, simdpp::int32>;
template <unsigned N>
using vnuint32 = VectorBase<N, uint32, simdpp::uint32>;

/* Precise types */
using vfloat  = vnfloat<PR_SIMD_BANDWIDTH>;
using vint32  = vnint32<PR_SIMD_BANDWIDTH>;
using vuint32 = vnuint32<PR_SIMD_BANDWIDTH>;
using bfloat  = simdpp::mask_float32<PR_SIMD_BANDWIDTH>;
using bint32  = simdpp::mask_int32<PR_SIMD_BANDWIDTH>;
using buint32 = bint32;

/* SFINAE Structure */
template <typename V>
struct VectorTemplate {
};

template <>
struct VectorTemplate<vfloat> {
	using float_t  = vfloat;
	using bool_t   = bfloat;
	using int32_t  = vint32;
	using uint32_t = vuint32;
};

template <>
struct VectorTemplate<float> {
	using float_t  = float;
	using bool_t   = bool;
	using int32_t  = int32;
	using uint32_t = uint32;
};

/* Variadic vector types */
template <typename T>
using Vector2t = Eigen::Matrix<T, 2, 1>;
template <typename T>
using Vector3t = Eigen::Matrix<T, 3, 1>;
template <typename T>
using Vector4t = Eigen::Matrix<T, 4, 1>;

/* Precise vector types */
using Vector2f  = Eigen::Vector2f;
using Vector2i  = Eigen::Vector2i;
using Vector2fv = Vector2t<vfloat>;
using Vector2iv = Vector2t<vint32>;

using Vector3f  = Eigen::Vector3f;
using Vector3i  = Eigen::Vector3i;
using Vector3fv = Vector3t<vfloat>;
using Vector3iv = Vector3t<vint32>;

using Vector4f  = Eigen::Vector4f;
using Vector4i  = Eigen::Vector4i;
using Vector4fv = Vector4t<vfloat>;
using Vector4iv = Vector4t<vint32>;

/* Utility functions */
inline float blend(float trueCase, float falseCase, bool mask) { return mask ? trueCase : falseCase; }
inline vfloat blend(vfloat trueCase, vfloat falseCase, bfloat mask) { return simdpp::blend(trueCase, falseCase, mask); }
inline vuint32 blend(vuint32 trueCase, vuint32 falseCase, buint32 mask) { return simdpp::blend(trueCase, falseCase, mask); }
inline vint32 blend(vint32 trueCase, vint32 falseCase, bint32 mask) { return simdpp::blend(trueCase, falseCase, mask); }

using std::signbit;
inline bfloat signbit(const vfloat& v) { return v < vfloat(0); }
inline bint32 signbit(const vint32& v) { return v < vint32(0); }

using std::abs;
inline vfloat abs(const vfloat& o) { return simdpp::abs(o); }
inline vint32 abs(const vint32& o) { return simdpp::abs(o); }

using std::sqrt;
inline vfloat sqrt(const vfloat& o) { return simdpp::sqrt(o); }

using std::sin;
template <unsigned N, class E>
inline vnfloat<N> sin(const simdpp::float32<N, E>& x)
{
	PR_SIMD_ALIGN
	float x_v[N];

	PR_SIMD_ALIGN
	float res_v[N];

	simdpp::store(x_v, x.eval());
	for (uint32 i = 0; i < N; ++i)
		res_v[i] = std::sin(x_v[i]);

	return simdpp::load(res_v);
}

using std::cos;
template <unsigned N, class E>
inline vnfloat<N> cos(const simdpp::float32<N, E>& x)
{
	PR_SIMD_ALIGN
	float x_v[N];

	PR_SIMD_ALIGN
	float res_v[N];

	simdpp::store(x_v, x.eval());
	for (uint32 i = 0; i < N; ++i)
		res_v[i] = std::cos(x_v[i]);

	return simdpp::load(res_v);
}

using std::tan;
template <unsigned N, class E>
inline vnfloat<N> tan(const simdpp::float32<N, E>& x)
{
	PR_SIMD_ALIGN
	float x_v[N];

	PR_SIMD_ALIGN
	float res_v[N];

	simdpp::store(x_v, x.eval());
	for (uint32 i = 0; i < N; ++i)
		res_v[i] = std::tan(x_v[i]);

	return simdpp::load(res_v);
}

using std::asin;
template <unsigned N, class E>
inline vnfloat<N> asin(const simdpp::float32<N, E>& x)
{
	PR_SIMD_ALIGN
	float x_v[N];

	PR_SIMD_ALIGN
	float res_v[N];

	simdpp::store(x_v, x.eval());
	for (uint32 i = 0; i < N; ++i)
		res_v[i] = std::asin(x_v[i]);

	return simdpp::load(res_v);
}

using std::acos;
template <unsigned N, class E>
inline vnfloat<N> acos(const simdpp::float32<N, E>& x)
{
	PR_SIMD_ALIGN
	float x_v[N];

	PR_SIMD_ALIGN
	float res_v[N];

	simdpp::store(x_v, x.eval());
	for (uint32 i = 0; i < N; ++i)
		res_v[i] = std::acos(x_v[i]);

	return simdpp::load(res_v);
}

using std::atan;
template <unsigned N, class E>
inline vnfloat<N> atan(const simdpp::float32<N, E>& x)
{
	PR_SIMD_ALIGN
	float x_v[N];

	PR_SIMD_ALIGN
	float res_v[N];

	simdpp::store(x_v, x.eval());
	for (uint32 i = 0; i < N; ++i)
		res_v[i] = std::atan(x_v[i]);

	return simdpp::load(res_v);
}

using std::atan2;
template <unsigned N, class E1, class E2>
inline vnfloat<N> atan2(const simdpp::float32<N, E1>& y,
						const simdpp::float32<N, E2>& x)
{
	PR_SIMD_ALIGN
	float y_v[N];
	PR_SIMD_ALIGN
	float x_v[N];

	PR_SIMD_ALIGN
	float res_v[N];

	simdpp::store(y_v, y.eval());
	simdpp::store(x_v, x.eval());
	for (uint32 i = 0; i < N; ++i)
		res_v[i] = std::atan2(y_v[i], x_v[i]);

	return simdpp::load(res_v);
}
} // namespace PR

// Eigen traits
namespace Eigen {
template <>
struct NumTraits<PR::vint32>
	: NumTraits<PR::int32> {
	typedef PR::vint32 Real;
	typedef PR::vint32 NonInteger;
	typedef PR::vint32 Nested;
	enum {
		IsComplex			  = 0,
		IsInteger			  = 1,
		IsSigned			  = 1,
		RequireInitialization = 0,
		ReadCost			  = 1,
		AddCost				  = 3,
		MulCost				  = 3
	};
};

template <>
struct NumTraits<PR::vuint32>
	: NumTraits<PR::uint32> {
	typedef PR::vuint32 Real;
	typedef PR::vuint32 NonInteger;
	typedef PR::vuint32 Nested;
	enum {
		IsComplex			  = 0,
		IsInteger			  = 1,
		IsSigned			  = 0,
		RequireInitialization = 0,
		ReadCost			  = 1,
		AddCost				  = 3,
		MulCost				  = 3
	};
};

template <>
struct NumTraits<PR::vfloat>
	: NumTraits<float> {
	typedef PR::vfloat Real;
	typedef PR::vfloat NonInteger;
	typedef PR::vfloat Nested;
	enum {
		IsComplex			  = 0,
		IsInteger			  = 0,
		IsSigned			  = 1,
		RequireInitialization = 0,
		ReadCost			  = 1,
		AddCost				  = 3,
		MulCost				  = 3
	};
};
} // namespace Eigen

// Utility functions for Vectors
// Has to be after the Eigen NumTraits declaration!
namespace PR {
inline Vector2fv promote(const Vector2f& o)
{
	return Vector2fv(fill_vector(o(0)), fill_vector(o(1)));
}

inline Vector3fv promote(const Vector3f& o)
{
	return Vector3fv(fill_vector(o(0)), fill_vector(o(1)), fill_vector(o(2)));
}

inline Vector4fv promote(const Vector4f& o)
{
	return Vector4fv(fill_vector(o(0)), fill_vector(o(1)),
					 fill_vector(o(2)), fill_vector(o(3)));
}
} // namespace PR