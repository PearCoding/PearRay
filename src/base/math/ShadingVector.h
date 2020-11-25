#pragma once

#include "PR_Config.h"

namespace PR {
// Vector in shading space with easy access to attributes
// Similar to PBRT
class PR_LIB_BASE ShadingVector {
public:
	ShadingVector() = default;
	/*implicit*/ inline ShadingVector(const Vector3f& v)
		: mVector(v)
	{
	}

	ShadingVector(const ShadingVector& other) = default;
	ShadingVector(ShadingVector&& other)	  = default;

	ShadingVector& operator=(const ShadingVector& other) = default;
	ShadingVector& operator=(ShadingVector&& other) = default;

	inline operator Vector3f() const { return mVector; }

	inline float theta() const { return std::acos(cosTheta()); }
	inline float phi() const
	{
		float v = std::atan2(mVector.y(), mVector.x());
		return v < 0 ? v + 2 * PR_PI : v;
	}

	inline float cosTheta() const { return mVector.z(); }
	inline float cos2Theta() const { return cosTheta() * cosTheta(); }
	inline float absCosTheta() const { return std::abs(cosTheta()); }

	inline float sinTheta() const { return std::sqrt(sin2Theta()); } // Costly
	inline float sin2Theta() const { return std::max(0.0f, 1 - cos2Theta()); }

	inline float tanTheta() const { return sinTheta() / cosTheta(); } // Costly
	inline float tan2Theta() const { return sin2Theta() / cos2Theta(); }

	inline float cosPhi() const
	{
		float v = sinTheta();
		return v < PR_EPSILON ? 0 : std::max(-1.0f, std::min(1.0f, mVector.x() / v));
	}

	inline float sinPhi() const
	{
		float v = sinTheta();
		return v < PR_EPSILON ? 0 : std::max(-1.0f, std::min(1.0f, mVector.y() / v));
	}

	inline float cos2Phi() const
	{
		float v = sin2Theta();
		return v < PR_EPSILON ? 0 : std::min(1.0f, mVector.x() * mVector.x() / v);
	}

	inline float sin2Phi() const
	{
		float v = sin2Theta();
		return v < PR_EPSILON ? 0 : std::min(1.0f, mVector.y() * mVector.y() / v);
	}

	inline float cosDiffPhi(const ShadingVector& other) const
	{
		float A = x() * x() + y() * y();
		float B = other.x() * other.x() + other.y() * other.y();

		if (A < PR_EPSILON || B < PR_EPSILON)
			return 0;

		return std::max(-1.0f, std::min(1.0f, (x() * other.x() + y() * other.y()) / std::sqrt(A * B)));
	}

	inline bool sameHemisphere(const ShadingVector& other) const { return cosTheta() * other.cosTheta() > 0.0f; }
	inline ShadingVector makeSameHemisphere(const ShadingVector& other) const
	{
		if (sameHemisphere(other))
			return other;
		else
			return other.flipZ();
	}
	inline ShadingVector makePositiveHemisphere() const
	{
		if (cosTheta() < 0)
			return flipZ();
		else
			return *this;
	}

	inline float x() const { return mVector.x(); }
	inline float y() const { return mVector.y(); }
	inline float z() const { return mVector.z(); }

	inline ShadingVector flipZ() const { return ShadingVector(Vector3f(mVector.x(), mVector.y(), -mVector.z())); }

	inline float operator[](size_t i) const { return mVector[i]; }
	inline float operator()(size_t i) const { return mVector[i]; }

	inline float& operator[](size_t i) { return mVector[i]; }
	inline float& operator()(size_t i) { return mVector[i]; }

	inline float dot(const ShadingVector& other) const { return mVector.dot(other.mVector); }

private:
	Vector3f mVector;
};

inline ShadingVector operator-(const ShadingVector& a)
{
	return ShadingVector(-(Vector3f)a);
}

inline ShadingVector operator+(const ShadingVector& a, const ShadingVector& b)
{
	return ShadingVector((Vector3f)a + (Vector3f)b);
}

inline ShadingVector operator-(const ShadingVector& a, const ShadingVector& b)
{
	return ShadingVector((Vector3f)a - (Vector3f)b);
}
} // namespace PR
