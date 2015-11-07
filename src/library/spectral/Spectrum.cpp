#include "Spectrum.h"

namespace PR
{
	Spectrum::Spectrum()
	{
		memset(mValues, 0, sizeof(float)*SAMPLING_COUNT);
	}

	Spectrum::~Spectrum()
	{
	}

	Spectrum::Spectrum(const Spectrum& spec)
	{
		memcpy(mValues, spec.mValues, sizeof(float)*SAMPLING_COUNT);
	}

	Spectrum& Spectrum::operator = (const Spectrum& spec)
	{
		memcpy(mValues, spec.mValues, sizeof(float)*SAMPLING_COUNT);
		return *this;
	}

	Spectrum Spectrum::operator + (const Spectrum& spec) const
	{
		Spectrum tmp = *this;
		tmp += spec;
		return tmp;
	}

	Spectrum Spectrum::operator + (float f) const
	{
		Spectrum tmp = *this;
		tmp += f;
		return tmp;
	}

	Spectrum& Spectrum::operator += (const Spectrum& spec)
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] += spec.mValues[i];
		}
		return *this;
	}

	Spectrum& Spectrum::operator += (float f)
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] += f;
		}
		return *this;
	}

	Spectrum Spectrum::operator - (const Spectrum& spec) const
	{
		Spectrum tmp = *this;
		tmp -= spec;
		return tmp;
	}

	Spectrum Spectrum::operator - (float f) const
	{
		Spectrum tmp = *this;
		tmp -= f;
		return tmp;
	}

	Spectrum& Spectrum::operator -= (const Spectrum& spec)
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] -= spec.mValues[i];
		}
		return *this;
	}

	Spectrum& Spectrum::operator -= (float f)
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] -= f;
		}
		return *this;
	}

	Spectrum Spectrum::operator * (const Spectrum& spec) const
	{
		Spectrum tmp = *this;
		tmp *= spec;
		return tmp;
	}

	Spectrum Spectrum::operator * (float f) const
	{
		Spectrum tmp = *this;
		tmp *= f;
		return tmp;
	}

	Spectrum& Spectrum::operator *= (const Spectrum& spec)
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] *= spec.mValues[i];
		}
		return *this;
	}

	Spectrum& Spectrum::operator *= (float f)
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] *= f;
		}
		return *this;
	}

	Spectrum Spectrum::operator / (const Spectrum& spec) const
	{
		Spectrum tmp = *this;
		tmp /= spec;
		return tmp;
	}

	Spectrum Spectrum::operator / (float f) const
	{
		Spectrum tmp = *this;
		tmp /= f;
		return tmp;
	}

	Spectrum& Spectrum::operator /= (const Spectrum& spec)
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] /= spec.mValues[i];
		}
		return *this;
	}

	Spectrum& Spectrum::operator /= (float f)
	{
		float sf = 1 / f;
		return (*this *= sf);
	}

	void Spectrum::setValue(uint32 index, float v)
	{
		mValues[index] = v;
	}

	float Spectrum::value(uint32 index) const
	{
		return mValues[index];
	}

	void Spectrum::setValueAtWavelength(float wavelength, float value)
	{
		if (wavelength < WAVELENGTH_START || wavelength > WAVELENGTH_END)
		{
			return;
		}

		uint32 i = (wavelength - WAVELENGTH_START) / WAVELENGTH_STEP;
		mValues[i] = value;
	}

	float Spectrum::approx(float wavelength, InterpolationType interpolation) const
	{
		if (wavelength < WAVELENGTH_START || wavelength > WAVELENGTH_END)
		{
			return -1;
		}

		float st = wavelength - WAVELENGTH_START;
		uint32 c = st / WAVELENGTH_STEP;
		uint32 m = fmodf(st, WAVELENGTH_STEP);

		if (st < 0 || c > SAMPLING_COUNT)
		{
			return 0;
		}

		if(interpolation == IT_Const)
		{ 
			if (m >= WAVELENGTH_STEP / 2 && c != SAMPLING_COUNT)
			{
				return mValues[c+1];
			}
			else
			{
				return mValues[c];
			}
		}
		else
		{
			if (c == SAMPLING_COUNT)
			{
				return mValues[c];
			}
			else
			{
				float t = m / (float)WAVELENGTH_STEP;
				return mValues[c] * (1 - t) + mValues[c + 1] * t;
			}
		}
	}

	float Spectrum::max() const
	{
		float h = 0;

		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			if (h < fabsf(mValues[i]))
			{
				h = fabsf(mValues[i]);
			}
		}

		return h;
	}

	float Spectrum::min() const
	{
		float h = std::numeric_limits<float>::max();

		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			if (h > fabsf(mValues[i]))
			{
				h = fabsf(mValues[i]);
			}
		}

		return h;
	}

	float Spectrum::avg() const
	{
		float h = 0;
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			h += mValues[i];
		}

		return h / SAMPLING_COUNT;
	}

	Spectrum& Spectrum::normalize()
	{
		float h = max();

		if (h != 0)
		{
			float sh = 1 / h;
			*this *= sh;
		}

		return *this;
	}

	Spectrum Spectrum::normalized() const
	{
		Spectrum spec = *this;
		return spec.normalize();
	}

	Spectrum& Spectrum::clamp(float start, float end)
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] = mValues[i] > end ? end : (mValues[i] < start ? start : mValues[i]);
		}

		return *this;
	}

	Spectrum Spectrum::clamped(float start, float end) const
	{
		Spectrum spec = *this;
		return spec.clamp(start, end);
	}

	Spectrum& Spectrum::lerp(const Spectrum& spec, float t)
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] = mValues[i] * (1-t) + spec.mValues[i] * t;
		}

		return *this;
	}

	Spectrum Spectrum::lerp(const Spectrum& spec1, const Spectrum& spec2, float t)
	{
		Spectrum spec = spec1;
		return spec.lerp(spec2, t);
	}

	Spectrum& Spectrum::sqrt()
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] = std::sqrtf(mValues[i]);
		}

		return *this;
	}

	Spectrum Spectrum::sqrted() const
	{
		Spectrum spec = *this;
		return spec.sqrt();
	}

	bool Spectrum::hasNaN() const
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			if (std::isnan(mValues[i]))
			{
				return true;
			}
		}

		return false;
	}

	bool Spectrum::hasInf() const
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			if (std::isinf(mValues[i]))
			{
				return true;
			}
		}

		return false;
	}

	bool Spectrum::isOnlyZero() const
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			if (mValues[i] != 0)
			{
				return false;
			}
		}

		return true;
	}

	// Global
	Spectrum operator + (float f, const Spectrum& spec)
	{
		return spec + f;
	}

	Spectrum operator - (float f, const Spectrum& spec)
	{
		Spectrum tmp;
		return (tmp - spec) + f;
	}

	Spectrum operator * (float f, const Spectrum& spec)
	{
		return spec * f;
	}

	Spectrum operator / (float f, const Spectrum& spec)
	{
		Spectrum tmp;
		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			tmp.setValue(i, f / spec.value(i));
		}

		return tmp;
	}
}