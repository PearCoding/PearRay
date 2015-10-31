#pragma once

#include "Config.h"

namespace PR
{
	enum InterpolationType
	{
		IT_Const,
		IT_Linear
	};

	class Spectrum
	{
	public:
		static const uint32 WAVELENGTH_START = 380;// nm
		static const uint32 WAVELENGTH_END = 780;// nm
		static const uint32 WAVELENGTH_STEP = 10;// nm
		static const uint32 SAMPLING_COUNT = (WAVELENGTH_END - WAVELENGTH_START) / WAVELENGTH_STEP;
		
		Spectrum();
		virtual ~Spectrum();

		void setValue(uint32 index, float v);
		float value(uint32 index) const;

		float approx(float wavelength, InterpolationType interpolation = IT_Linear) const;

		float max() const;// Amplitude
		float min() const;// Amplitude
		float avg() const;// General average

		void normalize();
	private:
		float mValues[SAMPLING_COUNT];
	};
}