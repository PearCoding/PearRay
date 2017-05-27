#pragma once

#include "PR_Config.h"

#include <algorithm>

namespace PR {
enum InterpolationType {
	IT_Const,
	IT_Linear
};

class PR_LIB Spectrum {
public:
	static constexpr uint32 WAVELENGTH_START	 = 380; // nm
	static constexpr uint32 WAVELENGTH_END		 = 780; // nm
	static constexpr uint32 WAVELENGTH_AREA_SIZE = (WAVELENGTH_END - WAVELENGTH_START);
	static constexpr uint32 WAVELENGTH_STEP		 = 5; // nm
	static constexpr uint32 SAMPLING_COUNT		 = WAVELENGTH_AREA_SIZE / WAVELENGTH_STEP + 1;
	static constexpr float ILL_SCALE			 = WAVELENGTH_STEP + 1;

	inline Spectrum();
	inline Spectrum(std::initializer_list<float> list);
	inline explicit Spectrum(float f);
	inline explicit Spectrum(const float* data); // Be cautious!
	inline ~Spectrum();

	Spectrum(const Spectrum& spec) = default;
	Spectrum(Spectrum&& spec)	  = default;
	Spectrum& operator=(const Spectrum& spec) = default;
	Spectrum& operator=(Spectrum&& spec) = default;

	inline Spectrum operator+(const Spectrum& spec) const;
	inline Spectrum& operator+=(const Spectrum& spec);

	inline Spectrum operator-(const Spectrum& spec) const;
	inline Spectrum& operator-=(const Spectrum& spec);

	inline Spectrum operator*(const Spectrum& spec) const; // Element wise
	inline Spectrum operator*(float f) const;
	inline Spectrum& operator*=(const Spectrum& spec);
	inline Spectrum& operator*=(float f);

	inline Spectrum operator/(const Spectrum& spec) const; // Element wise
	inline Spectrum operator/(float f) const;
	inline Spectrum& operator/=(const Spectrum& spec);
	inline Spectrum& operator/=(float f);

	inline void setValue(uint32 index, float v);
	inline float value(uint32 index) const;

	inline void setValueAtWavelength(float wavelength, float value);
	float approx(float wavelength, InterpolationType interpolation = IT_Linear) const;

	// Attention! Don't change values
	inline const float* ptr() const;

	inline void fill(float v);
	inline void fill(uint32 si, uint32 ei, float v);
	inline void clear();

	inline void copyTo(float* data) const;

	inline float max() const;	// Amplitude
	inline float min() const;	// Amplitude
	inline float avg() const;	// General average
	inline float sum() const;	// Sum
	inline float sqrSum() const; // Squared sum

	inline Spectrum& normalize();
	inline Spectrum normalized() const;

	inline Spectrum& clamp(float start = 0, float end = 1);
	inline Spectrum clamped(float start = 0, float end = 1) const;

	inline Spectrum& lerp(const Spectrum& spec, float t);
	inline static Spectrum lerp(const Spectrum& spec1, const Spectrum& spec2, float t);

	inline Spectrum& sqrt();
	inline Spectrum sqrted() const;

	inline bool hasNaN() const;
	inline bool hasInf() const;
	inline bool hasNegative() const;
	inline bool isOnlyZero() const;

	void weightPhotometric();
	float luminousFlux() const;

	static Spectrum fromBlackbody(float temp); // Temp in Kelvin (K), Output W·sr^−1·m^−3

private:
	float mValues[SAMPLING_COUNT];
};

inline Spectrum operator*(float f, const Spectrum& spec);
inline Spectrum operator/(float f, const Spectrum& spec);
inline bool operator==(const Spectrum& v1, const Spectrum& v2);
inline bool operator!=(const Spectrum& v1, const Spectrum& v2);
}

#include "Spectrum.inl"
