// IWYU pragma: private, include "spectral/EquidistantSpectrum.h"

namespace PR {
inline EquidistantSpectrumBase::EquidistantSpectrumBase(size_t sample_count, float wavelength_start, float wavelength_end)
	: mSampleCount(sample_count)
	, mWavelengthStart(wavelength_start)
	, mWavelengthEnd(wavelength_end)
	, mWavelengthDelta((wavelength_end - wavelength_start) / (sample_count - 1))
{
}

inline size_t EquidistantSpectrumBase::sampleCount() const { return mSampleCount; }
inline float EquidistantSpectrumBase::wavelengthStart() const { return mWavelengthStart; }
inline float EquidistantSpectrumBase::wavelengthEnd() const { return mWavelengthEnd; }
inline float EquidistantSpectrumBase::delta() const { return (mWavelengthEnd - mWavelengthStart) / (mSampleCount - 1); }

///////////////////////////////

inline EquidistantSpectrumView::EquidistantSpectrumView(const float* data, size_t sample_count, float wavelength_start, float wavelength_end)
	: EquidistantSpectrumBase(sample_count, wavelength_start, wavelength_end)
	, mData(data)
{
}

inline float EquidistantSpectrumView::at(size_t index) const
{
	PR_ASSERT(index < mSampleCount, "Expected valid index");
	return mData[index];
}

inline float EquidistantSpectrumView::operator[](size_t index) const { return at(index); }
inline const float* EquidistantSpectrumView::data() const { return mData; }

inline float EquidistantSpectrumView::lookup(float wavelength) const
{
	const float f	= std::min<float>(mSampleCount - 2, std::max(0.0f, (wavelength - mWavelengthStart) / mWavelengthDelta));
	const int index = f;
	const float t	= f - index;

	return mData[index] * (1 - t) + mData[index + 1] * t;
}

////////////////////////////////////
EquidistantSpectrum::RefData::RefData(float* data, bool external)
	: Data(data)
	, External(external)
{
}

EquidistantSpectrum::RefData::~RefData()
{
	if (!External && Data)
		delete[] Data;
}

inline EquidistantSpectrum::EquidistantSpectrum(float* data, size_t sample_count, float wavelength_start, float wavelength_end)
	: EquidistantSpectrumBase(sample_count, wavelength_start, wavelength_end)
	, mRef(std::make_shared<RefData>(data, true))
{
}

inline EquidistantSpectrum::EquidistantSpectrum(size_t sample_count, float wavelength_start, float wavelength_end)
	: EquidistantSpectrumBase(sample_count, wavelength_start, wavelength_end)
	, mRef(std::make_shared<RefData>(new float[sample_count], false))
{
}

inline float EquidistantSpectrum::at(size_t index) const
{
	PR_ASSERT(index < mSampleCount, "Expected valid index");
	return mRef->Data[index];
}

inline float& EquidistantSpectrum::at(size_t index)
{
	PR_ASSERT(index < mSampleCount, "Expected valid index");
	return mRef->Data[index];
}

inline float EquidistantSpectrum::operator[](size_t index) const { return at(index); }
inline float& EquidistantSpectrum::operator[](size_t index) { return at(index); }

inline const float* EquidistantSpectrum::data() const { return mRef->Data; }
inline float* EquidistantSpectrum::data() { return mRef->Data; }

inline float EquidistantSpectrum::lookup(float wavelength) const
{
	const float af	= std::max(0.0f, (wavelength - mWavelengthStart) / mWavelengthDelta);
	const int index = std::min<float>(mSampleCount - 2, af);
	const float t	= std::min<float>(mSampleCount - 1, af) - index;

	return mRef->Data[index] * (1 - t) + mRef->Data[index + 1] * t;
}

} // namespace PR
