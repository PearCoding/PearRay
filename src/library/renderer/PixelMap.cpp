#include "PixelMap.h"
#include "Renderer.h"
#include "DisplayDriver.h"

namespace PR
{
	PixelMap::PixelMap(Renderer* renderer, IDisplayDriver* driver) :
		mRenderer(renderer), mDriver(driver),
		mSamples(nullptr), mPixelError(nullptr)
	{
	}

	PixelMap::~PixelMap()
	{
		deinit();
	}
		
	void PixelMap::init()
	{
		PR_ASSERT(!mPixelError);
		PR_ASSERT(!mSamples);

		/* Setup Adaptive Sampling */
		if(mRenderer->settings().isAdaptiveSampling())
		{
			mPixelError = new float[mRenderer->renderWidth()*mRenderer->renderHeight()];
		}

		mSamples = new uint32[mRenderer->renderWidth()*mRenderer->renderHeight()];

		clear();
	}

	void PixelMap::deinit()
	{
		if(mSamples)
		{
			delete[] mSamples;
			mSamples = nullptr;
		}

		if(mPixelError)
		{
			delete[] mPixelError;
			mPixelError = nullptr;
		}
	}

	void PixelMap::clear()
	{
		std::fill_n(mSamples,
			mRenderer->renderHeight() * mRenderer->renderWidth(),
			0);

		if(mPixelError)
			std::fill_n(mPixelError,
				mRenderer->renderHeight() * mRenderer->renderWidth(),
				std::numeric_limits<float>::infinity());
	}

	void PixelMap::pushFragment(uint32 x, uint32 y, uint32 layer, const Spectrum& s)
	{
		if(isPixelFinished(x,y))
			return;

		uint32 dx = x - mRenderer->cropPixelOffsetX();
		uint32 dy = y - mRenderer->cropPixelOffsetY();
		const uint32 rw = mRenderer->renderWidth();
		//const uint32 rh = mRenderer->renderHeight();

		float t = 1.0f/(mSamples[dy*rw + dx] + 1);
	
		mDriver->pushFragment(x,y,layer, mDriver->getFragment(x,y,layer)*(1-t) + s*t);
		mSamples[dy*rw + dx]++;
	}

	Spectrum PixelMap::getFragment(uint32 x, uint32 y, uint32 layer) const
	{
		return mDriver->getFragment(x,y,layer);
	}

	void PixelMap::setSampleCount(uint32 x, uint32 y, uint32 sample)
	{
		uint32 dx = x - mRenderer->cropPixelOffsetX();
		uint32 dy = y - mRenderer->cropPixelOffsetY();

		mSamples[dy*mRenderer->renderWidth() + dx] = sample;
	}

	uint32 PixelMap::getSampleCount(uint32 x, uint32 y) const
	{
		uint32 dx = x - mRenderer->cropPixelOffsetX();
		uint32 dy = y - mRenderer->cropPixelOffsetY();

		return mSamples[dy*mRenderer->renderWidth() + dx];
	}

	void PixelMap::setPixelError(uint32 x, uint32 y, const Spectrum& pixel, const Spectrum& weight)
	{
		if(!mPixelError)
			return;

		uint32 dx = x - mRenderer->cropPixelOffsetX();
		uint32 dy = y - mRenderer->cropPixelOffsetY();

		const float err = std::abs((pixel - weight).max());
		mPixelError[dy*mRenderer->renderWidth() + dx] = err;	
	}

	bool PixelMap::isPixelFinished(uint32 x, uint32 y) const
	{
		if(mPixelError &&
			getSampleCount(x,y) >= mRenderer->settings().minPixelSampleCount())
		{
			uint32 dx = x - mRenderer->cropPixelOffsetX();
			uint32 dy = y - mRenderer->cropPixelOffsetY();

			return mPixelError[dy*mRenderer->renderWidth() + dx] <= mRenderer->settings().maxASError();
		}

		return false;
	}

	uint64 PixelMap::finishedPixelCount() const
	{
		const uint32 rw = mRenderer->renderWidth();
		const uint32 rh = mRenderer->renderHeight();
		const float minError = mRenderer->settings().maxASError();
		const uint32 minSamples = mRenderer->settings().minPixelSampleCount();

		uint64 pixelsFinished = 0;

		for(uint32 j = 0; j < rh; ++j)
		{
			for(uint32 i = 0; i < rw; ++i)
			{
				if(mSamples[j*rw + i] >= minSamples &&
					mPixelError[j*rw + i] <= minError)
					++pixelsFinished;
			}
		}

		return pixelsFinished;
	}
}