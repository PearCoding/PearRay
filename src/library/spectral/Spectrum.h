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
		static constexpr uint32 WAVELENGTH_START = 380;// nm
		static constexpr uint32 WAVELENGTH_END = 780;// nm
		static constexpr uint32 WAVELENGTH_STEP = 10;// nm
		static constexpr uint32 SAMPLING_COUNT = (WAVELENGTH_END - WAVELENGTH_START) / WAVELENGTH_STEP + 1;
		
		Spectrum();
		~Spectrum();

		Spectrum(const Spectrum& spec);
		Spectrum& operator = (const Spectrum& spec);

		Spectrum operator + (const Spectrum& spec) const;
		Spectrum operator + (float f) const;
		Spectrum& operator += (const Spectrum& spec);
		Spectrum& operator += (float f);

		Spectrum operator - (const Spectrum& spec) const;
		Spectrum operator - (float f) const;
		Spectrum& operator -= (const Spectrum& spec);
		Spectrum& operator -= (float f);

		Spectrum operator * (const Spectrum& spec) const;// Element wise
		Spectrum operator * (float f) const;
		Spectrum& operator *= (const Spectrum& spec);
		Spectrum& operator *= (float f);

		Spectrum operator / (const Spectrum& spec) const;// Element wise
		Spectrum operator / (float f) const;
		Spectrum& operator /= (const Spectrum& spec);
		Spectrum& operator /= (float f);

		void setValue(uint32 index, float v);
		float value(uint32 index) const;

		void setValueAtWavelength(float wavelength, float value);
		float approx(float wavelength, InterpolationType interpolation = IT_Linear) const;

		float max() const;// Amplitude
		float min() const;// Amplitude
		float avg() const;// General average

		Spectrum& normalize();
		Spectrum normalized() const;

		Spectrum& clamp(float start, float end);
		Spectrum clamped(float start, float end) const;

		Spectrum& lerp(const Spectrum& spec, float t);
		static Spectrum lerp(const Spectrum& spec1, const Spectrum& spec2, float t);

		Spectrum& sqrt();
		Spectrum sqrted() const;

		bool hasNaN() const;
		bool hasInf() const;
		bool isOnlyZero() const;
	private:
		float mValues[SAMPLING_COUNT];
	};

	Spectrum operator + (float f, const Spectrum& spec);
	Spectrum operator - (float f, const Spectrum& spec);
	Spectrum operator * (float f, const Spectrum& spec);
	Spectrum operator / (float f, const Spectrum& spec);
}