#include "ColorBuffer.h"
#include "spectral/ToneMapper.h"
#include <algorithm>

namespace PR {
ColorBuffer::_Data::_Data(size_t width, size_t height, ColorBufferMode mode)
	: Width(width)
	, Height(height)
	, Mode(mode)
	, Ptr(nullptr)
{
	size_t elems = (mode == ColorBufferMode::RGB ? 3 : 4);
	Ptr			 = new float[width * height * elems];

	std::fill_n(Ptr, width * height * elems, 1.0f);
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

void ColorBuffer::map(const ToneMapper& mapper,
					  const float* specIn, const float* weightIn)
{
	PR_ASSERT(mData, "Expected valid buffer");
	mapper.map(specIn, weightIn, mData->Ptr, channels(), mData->Width * mData->Height);
}

void ColorBuffer::flipY()
{
	PR_ASSERT(mData, "Expected valid buffer");
	const int h		= (int)height();
	const int elems = (int)heightPitch();
	for (int i = 0; i < h / 2; ++i) {
		float* ptr1 = &mData->Ptr[i * elems];
		float* ptr2 = &mData->Ptr[(h - i - 1) * elems];
		std::swap_ranges(ptr1, ptr1 + elems, ptr2);
	}
}
} // namespace PR
