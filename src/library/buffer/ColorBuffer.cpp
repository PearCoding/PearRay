#include "ColorBuffer.h"
#include "spectral/Spectrum.h"
#include "spectral/ToneMapper.h"

namespace PR {
ColorBuffer::_Data::_Data(uint32 width, uint32 height, ColorBufferMode mode)
	: Width(width)
	, Height(height)
	, Mode(mode)
	, Ptr(nullptr)
{
	size_t elems = (mode == CBM_RGB ? 3 : 4);
	Ptr			 = new float[width * height * elems];

	std::fill_n(Ptr, width*height*elems, 1);
}

ColorBuffer::_Data::~_Data()
{
	delete[] Ptr;
}

ColorBuffer::ColorBuffer(uint32 width, uint32 height, ColorBufferMode mode)
	: mData(std::make_shared<_Data>(width, height, mode))
{
}

ColorBuffer::ColorBuffer(const ColorBuffer& other)
	: mData(other.mData)
{
}

ColorBuffer::ColorBuffer(ColorBuffer&& other)
	: mData(std::move(other.mData))
{
}

ColorBuffer::~ColorBuffer()
{
}

ColorBuffer& ColorBuffer::operator=(const ColorBuffer& other)
{
	mData = other.mData;
	return *this;
}

ColorBuffer& ColorBuffer::operator=(ColorBuffer&& other)
{
	mData = std::move(other.mData);
	return *this;
}

void ColorBuffer::map(const ToneMapper& mapper, const float* specIn, uint32 samples)
{
	mapper.map(specIn, mData->Ptr, samples, mData->Mode == CBM_RGB ? 3 : 4);
}

void ColorBuffer::mapOnlyMapper(const ToneMapper& mapper, const float* rgbIn)
{
	mapper.mapOnlyMapper(rgbIn, mData->Ptr, mData->Mode == CBM_RGB ? 3 : 4);
}
}
