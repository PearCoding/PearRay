#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB_CORE EquidistantSpectrumBase {
public:
	inline EquidistantSpectrumBase(size_t sample_count, float wavelength_start, float wavelength_end);

	inline size_t sampleCount() const;
	inline float wavelengthStart() const;
	inline float wavelengthEnd() const;

protected:
	const size_t mSampleCount;
	const float mWavelengthStart;
	const float mWavelengthEnd;
	const float mWavelengthDelta;
};

class PR_LIB_CORE EquidistantSpectrumView : public EquidistantSpectrumBase {
public:
	inline EquidistantSpectrumView(const float* data, size_t sample_count, float wavelength_start, float wavelength_end);
	~EquidistantSpectrumView() = default;

	inline float at(size_t index) const;
	inline float operator[](size_t index) const;
	inline const float* data() const;

	inline float lookup(float wavelength) const;

private:
	const float* mData;
};

class PR_LIB_CORE EquidistantSpectrum : public EquidistantSpectrumBase {
public:
	inline EquidistantSpectrum(float* data, size_t sample_count, float wavelength_start, float wavelength_end);
	inline EquidistantSpectrum(size_t sample_count, float wavelength_start, float wavelength_end);
	~EquidistantSpectrum() = default;

	EquidistantSpectrum(const EquidistantSpectrum& other) = default;
	EquidistantSpectrum(EquidistantSpectrum&& other)	  = default;

	EquidistantSpectrum& operator=(const EquidistantSpectrum& other) = default;
	EquidistantSpectrum& operator=(EquidistantSpectrum&& other) = default;

	inline float at(size_t index) const;
	inline float& at(size_t index);
	inline float operator[](size_t index) const;
	inline float& operator[](size_t index);
	inline const float* data() const;
	inline float* data();

	inline float lookup(float wavelength) const;

private:
	struct RefData {
		float* Data;
		bool External;

		inline RefData(float* data, bool external);
		inline ~RefData();
	};
	std::shared_ptr<RefData> mRef;
};
} // namespace PR

#include "EquidistantSpectrum.inl"