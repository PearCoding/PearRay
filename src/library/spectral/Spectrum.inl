namespace PR
{
	inline Spectrum::Spectrum() :
		mEmissive(false)
	{
		std::memset(mValues, 0, sizeof(float)*SAMPLING_COUNT);
	}

	inline Spectrum::Spectrum(float f) :
		mEmissive(false)
	{
		fill(f);
	}

	inline Spectrum::Spectrum(const float* data) :
		mEmissive(false)
	{
		std::memcpy(mValues, data, sizeof(float)*SAMPLING_COUNT);
	}

	inline Spectrum::~Spectrum()
	{
	}

	inline Spectrum::Spectrum(const Spectrum& spec)
	{
		std::memcpy(mValues, spec.mValues, sizeof(float)*SAMPLING_COUNT);
		mEmissive = spec.mEmissive;
	}

	inline Spectrum& Spectrum::operator = (const Spectrum& spec)
	{
		std::memcpy(mValues, spec.mValues, sizeof(float)*SAMPLING_COUNT);
		mEmissive = spec.mEmissive;
		return *this;
	}

	inline Spectrum Spectrum::operator + (const Spectrum& spec) const
	{
		Spectrum tmp = *this;
		tmp += spec;
		return tmp;
	}

	inline Spectrum& Spectrum::operator += (const Spectrum& spec)
	{
		const float* o = spec.mValues;
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
			mValues[i] += o[i];

		return *this;
	}

	inline Spectrum Spectrum::operator - (const Spectrum& spec) const
	{
		Spectrum tmp = *this;
		tmp -= spec;
		return tmp;
	}

	inline Spectrum& Spectrum::operator -= (const Spectrum& spec)
	{
		const float* o = spec.mValues;
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
			mValues[i] -= o[i];

		return *this;
	}

	inline Spectrum Spectrum::operator * (const Spectrum& spec) const
	{
		Spectrum tmp = *this;
		tmp *= spec;
		return tmp;
	}

	inline Spectrum Spectrum::operator * (float f) const
	{
		Spectrum tmp = *this;
		tmp *= f;
		return tmp;
	}

	inline Spectrum& Spectrum::operator *= (const Spectrum& spec)
	{
		const float* o = spec.mValues;
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
			mValues[i] *= o[i];

		return *this;
	}

	inline Spectrum& Spectrum::operator *= (float f)
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
			mValues[i] *= f;
		return *this;
	}

	inline Spectrum Spectrum::operator / (const Spectrum& spec) const
	{
		Spectrum tmp = *this;
		tmp /= spec;
		return tmp;
	}

	inline Spectrum Spectrum::operator / (float f) const
	{
		Spectrum tmp = *this;
		tmp /= f;
		return tmp;
	}

	inline Spectrum& Spectrum::operator /= (const Spectrum& spec)
	{
		const float* o = spec.mValues;
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
			mValues[i] /= o[i];
		return *this;
	}

	inline Spectrum& Spectrum::operator /= (float f)
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
			mValues[i] /= f;
		return *this;
	}

	inline void Spectrum::setValue(uint32 index, float v)
	{
		mValues[index] = v;
	}

	inline float Spectrum::value(uint32 index) const
	{
		return mValues[index];
	}

#ifndef PR_NO_SPECTRAL
	inline void Spectrum::setValueAtWavelength(float wavelength, float value)
	{
		if (wavelength < WAVELENGTH_START || wavelength > WAVELENGTH_END)
			return;

		uint32 i = (uint32)((wavelength - WAVELENGTH_START) / WAVELENGTH_STEP);
		mValues[i] = value;
	}
#endif

	inline void Spectrum::fill(float v)
	{
		std::fill_n(mValues, SAMPLING_COUNT, v);
	}

	inline void Spectrum::clear()
	{
		std::memset(mValues, 0, sizeof(float)*SAMPLING_COUNT);
	}

	inline float Spectrum::max() const
	{
		float h = 0;

		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			if (h < std::abs(mValues[i]))
			{
				h = std::abs(mValues[i]);
			}
		}

		return h;
	}

	inline float Spectrum::min() const
	{
		float h = std::numeric_limits<float>::max();

		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			if (h > std::abs(mValues[i]))
			{
				h = std::abs(mValues[i]);
			}
		}

		return h;
	}

	inline float Spectrum::avg() const
	{
		float h = 0;
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			h += mValues[i];
		}

		return h / SAMPLING_COUNT;
	}

	inline Spectrum& Spectrum::normalize()
	{
		float h = max();

		if (h > PM_EPSILON)
		{
			float sh = 1 / h;
			*this *= sh;
		}

		return *this;
	}

	inline Spectrum Spectrum::normalized() const
	{
		Spectrum spec = *this;
		return spec.normalize();
	}

	inline Spectrum& Spectrum::clamp(float start, float end)
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] = mValues[i] > end ? end : (mValues[i] < start ? start : mValues[i]);
		}

		return *this;
	}

	inline Spectrum Spectrum::clamped(float start, float end) const
	{
		Spectrum spec = *this;
		return spec.clamp(start, end);
	}

	inline Spectrum& Spectrum::lerp(const Spectrum& spec, float t)
	{
		const float* o = spec.mValues;
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] = mValues[i] * (1-t) + o[i] * t;
		}

		return *this;
	}

	inline Spectrum Spectrum::lerp(const Spectrum& spec1, const Spectrum& spec2, float t)
	{
		Spectrum spec = spec1;
		return spec.lerp(spec2, t);
	}

	inline Spectrum& Spectrum::sqrt()
	{
		for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		{
			mValues[i] = std::sqrt(mValues[i]);
		}

		return *this;
	}

	inline Spectrum Spectrum::sqrted() const
	{
		Spectrum spec = *this;
		return spec.sqrt();
	}

	inline bool Spectrum::hasNaN() const
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

	inline bool Spectrum::hasInf() const
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

	inline bool Spectrum::isOnlyZero() const
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

	inline bool Spectrum::isEmissive() const
	{
		return mEmissive;
	}

	inline void Spectrum::setEmissive(bool b)
	{
		mEmissive = b;
	}

	// Global
	inline Spectrum operator * (float f, const Spectrum& spec)
	{
		return spec * f;
	}

	inline Spectrum operator / (float f, const Spectrum& spec)
	{
		Spectrum tmp;
		tmp.setEmissive(spec.isEmissive());

		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			tmp.setValue(i, f / spec.value(i));
		}

		return tmp;
	}
}