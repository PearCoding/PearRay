#include "ColorBuffer.h"
#include "spectral/Spectrum.h"
#include "spectral/ToneMapper.h"

namespace PR {
ColorBuffer::_Data::_Data(size_t width, size_t height, ColorBufferMode mode)
	: Width(width)
	, Height(height)
	, Mode(mode)
	, Ptr(nullptr)
{
	size_t elems = (mode == CBM_RGB ? 3 : 4);
	Ptr			 = new float[width * height * elems];

	std::fill_n(Ptr, width * height * elems, 1);
}

ColorBuffer::_Data::~_Data()
{
	delete[] Ptr;
}

ColorBuffer::ColorBuffer(size_t width, size_t height, ColorBufferMode mode)
	: mData(std::make_shared<_Data>(width, height, mode))
{
}

ColorBuffer::~ColorBuffer()
{
}

void ColorBuffer::map(const ToneMapper& mapper, const float* specIn, size_t samples)
{
	mapper.map(specIn, samples,
			   mData->Ptr, channels(), mData->Width * mData->Height);
}

void ColorBuffer::mapOnlyMapper(const ToneMapper& mapper, const float* rgbIn)
{
	mapper.mapOnlyMapper(rgbIn, mData->Ptr,
						 channels(), mData->Width * mData->Height);
}
} // namespace PR
