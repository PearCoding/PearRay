#pragma once

#include "PR_Config.h"

namespace PR {
// Vector in shading space with easy access to attributes
// Similar to PBRT
class PR_LIB_BASE ShadingVector : public Vector3f {
public:
	typedef Vector3f Base;

	// This constructor allows you to construct ShadingVector from Eigen expressions
	template <typename OtherDerived>
	ShadingVector(const Eigen::MatrixBase<OtherDerived>& other)
		: Vector3f(other)
	{
	}
	// This method allows you to assign Eigen expressions to ShadingVector
	template <typename OtherDerived>
	ShadingVector& operator=(const Eigen::MatrixBase<OtherDerived>& other)
	{
		this->Base::operator=(other);
		return *this;
	}

	ShadingVector() = default;

	ShadingVector(const ShadingVector& other) = default;
	ShadingVector(ShadingVector&& other)	  = default;

	ShadingVector& operator=(const ShadingVector& other) = default;
	ShadingVector& operator=(ShadingVector&& other) = default;

	inline float theta() const { return std::acos(cosTheta()); }
	inline float phi() const
	{
		float v = std::atan2(y(), x());
		return v < 0 ? v + 2 * PR_PI : v;
	}

	inline float cosTheta() const { return z(); }
	inline float cos2Theta() const { return cosTheta() * cosTheta(); }
	inline float absCosTheta() const { return std::abs(cosTheta()); }

	inline float sinTheta() const { return std::sqrt(sin2Theta()); } // Costly
	inline float sin2Theta() const { return std::max(0.0f, 1 - cos2Theta()); }

	inline float tanTheta() const { return absCosTheta() <= PR_EPSILON ? 0 : sinTheta() / cosTheta(); } // Costly
	inline float tan2Theta() const { return absCosTheta() <= PR_EPSILON ? 0 : sin2Theta() / cos2Theta(); }

	inline float cosPhi() const
	{
		float v = sinTheta();
		return std::abs(v) <= PR_EPSILON ? 0 : std::max(-1.0f, std::min(1.0f, x() / v));
	}

	inline float sinPhi() const
	{
		float v = sinTheta();
		return std::abs(v) <= PR_EPSILON ? 0 : std::max(-1.0f, std::min(1.0f, y() / v));
	}

	inline float cos2Phi() const
	{
		float v = sin2Theta();
		return v <= PR_EPSILON ? 0 : std::min(1.0f, x() * x() / v);
	}

	inline float sin2Phi() const
	{
		float v = sin2Theta();
		return v <= PR_EPSILON ? 0 : std::min(1.0f, y() * y() / v);
	}

	inline float cosDiffPhi(const ShadingVector& other) const
	{
		const float A  = x() * x() + y() * y();
		const float B  = other.x() * other.x() + other.y() * other.y();
		const float AB = A * B;

		if (AB <= PR_EPSILON)
			return 0;

		return std::max(-1.0f, std::min(1.0f, (x() * other.x() + y() * other.y()) / std::sqrt(AB)));
	}

	inline bool sameHemisphere(const ShadingVector& other) const { return std::signbit(cosTheta()) == std::signbit(other.cosTheta()); }
	inline bool isPositiveHemisphere() const { return !std::signbit(cosTheta()); }

	/// Make other the same hemisphere as this one
	inline ShadingVector makeSameHemisphere(const ShadingVector& other) const
	{
		return sameHemisphere(other) ? other : -other;
	}

	/// Return transformed vector if not in positive hemisphere, else this will be returned
	inline ShadingVector makePositiveHemisphere() const
	{
		return isPositiveHemisphere() ? *this : -(*this);
	}

	inline float x() const { return (*this)[0]; }
	inline float y() const { return (*this)[1]; }
	inline float z() const { return (*this)[2]; }
};
} // namespace PR
