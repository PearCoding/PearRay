#include "DisplayBuffer.h"
#include "renderer/Renderer.h"

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
		PR_ASSERT(!mData);

		mRenderer = renderer;
		mData = new float[renderer->width()*renderer->height()*Spectrum::SAMPLING_COUNT];

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
}