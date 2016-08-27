#include "DisplayBuffer.h"
#include "renderer/Renderer.h"

#include "spectral/ToneMapper.h"

#include <OpenImageIO/imageio.h>

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

OIIO_NAMESPACE_USING;

namespace PRU
{
	using namespace PR;

	DisplayBuffer::DisplayBuffer() :
		mData(nullptr), mRenderer(nullptr), mSaveData(nullptr)
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

			if(mSaveData)
			{
				delete[] mSaveData;
				mSaveData = nullptr;
			}
		}

		mRenderer = renderer;

		if(!mData)
		{
			mData = new float[renderer->width()*renderer->height()*Spectrum::SAMPLING_COUNT];
			mSaveData = new PR::uint8[renderer->width()*renderer->height()*3];
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

		if(mSaveData)
		{
			delete[] mSaveData;
			mSaveData = nullptr;
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

	bool DisplayBuffer::save(const PR::ToneMapper& toneMapper, const std::string& file) const
	{
		PR_ASSERT(toneMapper.isByteMode());
		
		ImageSpec spec(mRenderer->width(), mRenderer->height(), 3, TypeDesc::UINT8);
		std::memset(mSaveData, 0, mRenderer->width() * mRenderer->height() * 3);
		toneMapper.exec(mData, mSaveData);
		
		// try
		// {
		// 	boost::interprocess::file_lock flock(file.c_str());
		// 	boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(flock);

			ImageOutput* out = ImageOutput::create(file);
			if(!out)
				return false;
			
			out->open(file, spec);
			out->write_image(TypeDesc::UINT8, mSaveData);
			/*for (uint32 y = 0; y < mRenderer->height(); ++y)
				out->write_scanline(y,0, TypeDesc::UINT8, &mSaveData[y*mRenderer->width()*3]);*/
			out->close();
			ImageOutput::destroy(out);
		// }
		// catch(const boost::interprocess::interprocess_exception& e)
		// {
		// 	ImageOutput* out = ImageOutput::create(file);// Just create it.
		// 	ImageOutput::destroy(out);
		// 	return false;
		// }

		return true;
	}
}