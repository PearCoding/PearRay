#include "DisplayBuffer.h"
#include "renderer/Renderer.h"

#include "spectral/RGBConverter.h"

#include <OpenImageIO/imageio.h>

OIIO_NAMESPACE_USING;

namespace PRU
{
	using namespace PR;

	DisplayBuffer::DisplayBuffer() :
		mData(nullptr), mRenderer(nullptr)
	{
	}

	DisplayBuffer::~DisplayBuffer()
	{
		deinit();
	}

	void DisplayBuffer::init(Renderer* renderer)
	{
		PR_ASSERT(renderer);

		// Delete if resolution changed.
		if(mRenderer && mData &&
			(mRenderer->width() != renderer->width() || mRenderer->height() != renderer->height()))
		{
			delete[] mData;
			mData = nullptr;
		}

		mRenderer = renderer;

		if(!mData)
		{
			mData = new float[renderer->width()*renderer->height()*Spectrum::SAMPLING_COUNT];
		}

		clear();
	}

	void DisplayBuffer::deinit()
	{
		if (mData)
		{
			delete[] mData;
			mData = nullptr;
		}
	}

	// TODO: No layer support
	void DisplayBuffer::pushFragment(uint32 x, uint32 y, uint32 layer, uint32 sample, const Spectrum& s)
	{
		if (!mData)
			return;

		PR_ASSERT(y < mRenderer->height());
		PR_ASSERT(x < mRenderer->width());

		const uint32 index = y*mRenderer->width()*Spectrum::SAMPLING_COUNT 
			+ x*Spectrum::SAMPLING_COUNT;

		const float t = 1.0f / (sample + 1.0f);
		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			mData[index + i] = mData[index + i] * (1-t) + s.value(i) * t;
		}
	}

	Spectrum DisplayBuffer::fragment(uint32 x, uint32 y, uint32 layer) const
	{
		if (!mData)
			return Spectrum();

		PR_ASSERT(y < mRenderer->height());
		PR_ASSERT(x < mRenderer->width());

		const uint32 index = y*mRenderer->width()*Spectrum::SAMPLING_COUNT
			+ x*Spectrum::SAMPLING_COUNT;

		return Spectrum(&mData[index]);
	}

	void DisplayBuffer::clear()
	{
		if(mData)
			std::memset(mData, 0, mRenderer->width()*mRenderer->height()*Spectrum::SAMPLING_COUNT * sizeof(float));
	}

	float* DisplayBuffer::ptr() const
	{
		return mData;
	}

	bool DisplayBuffer::save(const std::string& file) const
	{
		unsigned char* rgb = new unsigned char[mRenderer->width() * mRenderer->height() * 3];
		std::memset(rgb, 0, mRenderer->width() * mRenderer->height() * 3);

		for(uint32 y = mRenderer->cropPixelOffsetY();
			y < mRenderer->cropPixelOffsetY() + mRenderer->renderHeight();
			++y)
		{
			for(uint32 x = mRenderer->cropPixelOffsetX();
				x < mRenderer->cropPixelOffsetX() + mRenderer->renderWidth();
				++x)
			{
				float r, g, b;
				PR::RGBConverter::convert(fragment(x,y,0), r,g,b);
				//PR::RGBConverter::gamma(r,g,b);

				rgb[y*mRenderer->width()*3 + x*3] = static_cast<PR::uint8>(r*255);
				rgb[y*mRenderer->width()*3 + x*3 + 1] = static_cast<PR::uint8>(g*255);
				rgb[y*mRenderer->width()*3 + x*3 + 2] = static_cast<PR::uint8>(b*255);
			}
		}

		ImageOutput* out = ImageOutput::create(file);
		if(!out)
			return false;
		
		ImageSpec spec(mRenderer->width(), mRenderer->height(), 3, TypeDesc::UINT8);
		out->open(file, spec);
		out->write_image(TypeDesc::UINT8, rgb);
		out->close();
		ImageOutput::destroy(out);

		delete[] rgb;

		return true;
	}
}