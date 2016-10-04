#include "DisplayBuffer.h"
#include "renderer/Renderer.h"
#include "Logger.h"

#include "spectral/ToneMapper.h"

#include <OpenImageIO/imageio.h>

#include <boost/filesystem.hpp>

OIIO_NAMESPACE_USING;

namespace PRU
{
	using namespace PR;

	DisplayBuffer::DisplayBuffer() :
		mData(nullptr), mSaveData(nullptr), mRenderer(nullptr)
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

		if(mRenderer)
		{
			// Setup lock directory
			if(!boost::filesystem::create_directory(mRenderer->workingDir() + "/.lock"))
				PR_LOGGER.logf(L_Warning, M_System,
					"Couldn't create lock directory '%s/.lock'. Maybe already running?", mRenderer->workingDir().c_str());

			if(!boost::filesystem::remove(mRenderer->workingDir() + "/.img_lock"))// Remove it now
				PR_LOGGER.logf(L_Error, M_System,
					"Couldn't delete lock directory '%s/.img_lock'!", mRenderer->workingDir().c_str());
		}

		if(!mData)
		{
			mData = new float[renderer->width()*renderer->height()*Spectrum::SAMPLING_COUNT];
			mSaveData = new PR::uint8[renderer->width()*renderer->height()*3];
		}

		clear(0,0,0,0);
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

		if(!mRenderer)
			return;

		if(!boost::filesystem::remove(mRenderer->workingDir() + "/.lock"))
			PR_LOGGER.logf(L_Error, M_System,
				"Couldn't delete lock directory '%s/.lock'!", mRenderer->workingDir().c_str());
	}

	// TODO: No layer support
	void DisplayBuffer::pushFragment(uint32 x, uint32 y, uint32 layer, const Spectrum& s)
	{
		if (!mData)
			return;

		PR_ASSERT(y < mRenderer->height());
		PR_ASSERT(x < mRenderer->width());

		const uint32 index = y*mRenderer->width()*Spectrum::SAMPLING_COUNT 
			+ x*Spectrum::SAMPLING_COUNT;

		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
			mData[index + i] = s.value(i);
	}

	Spectrum DisplayBuffer::getFragment(uint32 x, uint32 y, uint32 layer) const
	{
		if (!mData)
			return Spectrum();

		PR_ASSERT(y < mRenderer->height());
		PR_ASSERT(x < mRenderer->width());

		const uint32 index = y*mRenderer->width()*Spectrum::SAMPLING_COUNT
			+ x*Spectrum::SAMPLING_COUNT;

		return Spectrum(&mData[index]);
	}

	void DisplayBuffer::clear(uint32 sx, uint32 sy, uint32 ex, uint32 ey)
	{
		if(!mData)
			return;
		
		if(sx == 0 && sy == 0 &&
			(ex == 0 || ex == mRenderer->width()) &&
			(ey == 0 || ey == mRenderer->height()))// Full clear
		{
			std::memset(mData, 0, mRenderer->width()*mRenderer->height()*Spectrum::SAMPLING_COUNT * sizeof(float));
		}
		else
		{
			PR_ASSERT(sx < ex);
			PR_ASSERT(sy < ey);

			for(uint32 y = sy; y < ey; ++y)// Line by line
			{
				std::memset(&mData[y*Spectrum::SAMPLING_COUNT + sx], 0,
					(ex - sx)*Spectrum::SAMPLING_COUNT * sizeof(float));
			}
		}
	}

	float* DisplayBuffer::ptr() const
	{
		return mData;
	}

	bool DisplayBuffer::save(const PR::ToneMapper& toneMapper, const std::string& file, bool force) const
	{
		PR_ASSERT(toneMapper.isByteMode());
		
		ImageSpec spec(mRenderer->width(), mRenderer->height(), 3, TypeDesc::UINT8);
		std::memset(mSaveData, 0, mRenderer->width() * mRenderer->height() * 3);
		toneMapper.exec(mData, mSaveData);
		
		if(!force && !boost::filesystem::create_directory(mRenderer->workingDir() + "/.img_lock"))
			return true;// Just don't produce error information
		
		ImageOutput* out = ImageOutput::create(file);
		if(!out)
			return false;
		
		out->open(file, spec);
		out->write_image(TypeDesc::UINT8, mSaveData);
		out->close();
		ImageOutput::destroy(out);

		if(!force && !boost::filesystem::remove(mRenderer->workingDir() + "/.img_lock"))// Remove it now
			PR_LOGGER.logf(L_Error, M_System,
				"Couldn't delete lock directory '%s/.img_lock'!", mRenderer->workingDir().c_str());

		return true;
	}
}