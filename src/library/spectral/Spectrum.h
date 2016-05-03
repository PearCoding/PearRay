#pragma once

#include "Config.h"

namespace PR
{
	enum InterpolationType
	{
		IT_Const,
		IT_Linear
	};

	class PR_LIB Spectrum
	{
	public:
		static constexpr uint32 WAVELENGTH_START = 360;// nm [old 380]
		static constexpr uint32 WAVELENGTH_END = 800;// nm [old 780]
		static constexpr uint32 WAVELENGTH_AREA_SIZE = (WAVELENGTH_END - WAVELENGTH_START);
		static constexpr uint32 WAVELENGTH_STEP = 5;// nm [old 10]
		static constexpr uint32 SAMPLING_COUNT = WAVELENGTH_AREA_SIZE / WAVELENGTH_STEP + 1;
		
		inline Spectrum();
		inline Spectrum(const float* data);// Be cautious!
		inline ~Spectrum();

		inline Spectrum(const Spectrum& spec);
		inline Spectrum& operator = (const Spectrum& spec);

		inline Spectrum operator + (const Spectrum& spec) const;
		inline Spectrum operator + (float f) const;
		inline Spectrum& operator += (const Spectrum& spec);
		inline Spectrum& operator += (float f);

		inline Spectrum operator - (const Spectrum& spec) const;
		inline Spectrum operator - (float f) const;
		inline Spectrum& operator -= (const Spectrum& spec);
		inline Spectrum& operator -= (float f);

		inline Spectrum operator * (const Spectrum& spec) const;// Element wise
		inline Spectrum operator * (float f) const;
		inline Spectrum& operator *= (const Spectrum& spec);
		inline Spectrum& operator *= (float f);

		inline Spectrum operator / (const Spectrum& spec) const;// Element wise
		inline Spectrum operator / (float f) const;
		inline Spectrum& operator /= (const Spectrum& spec);
		inline Spectrum& operator /= (float f);

		inline void setValue(uint32 index, float v);
		inline float value(uint32 index) const;

		inline void setValueAtWavelength(float wavelength, float value);
		float approx(float wavelength, InterpolationType interpolation = IT_Linear) const;

		inline float max() const;// Amplitude
		inline float min() const;// Amplitude
		inline float avg() const;// General average

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
		inline bool isOnlyZero() const;

		inline bool isEmissive() const;
		inline void setEmissive(bool b);

		static Spectrum fromBlackbody(float temp);// Temp in Kelvin (K)
		static Spectrum fromBlackbodyNorm(float temp);
	private:
		float mValues[SAMPLING_COUNT];
		bool mEmissive;
	};

	inline Spectrum operator + (float f, const Spectrum& spec);
	inline Spectrum operator - (float f, const Spectrum& spec);
	inline Spectrum operator * (float f, const Spectrum& spec);
	inline Spectrum operator / (float f, const Spectrum& spec);
}

#include "Spectrum.inl"