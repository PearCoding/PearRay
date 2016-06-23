#include "RenderResult.h"

namespace PR
{
	RenderResult::RenderResult(uint32 w, uint32 h) :
		mData(nullptr)
	{
		mData = new InternalData;
		mData->Width = w;
		mData->Height = h;
		mData->Data = new float[mData->Width*mData->Height*Spectrum::SAMPLING_COUNT];
		mData->RefCounter = 1;

		clear();
	}

	RenderResult::RenderResult(const RenderResult& res)
	{
		mData = res.mData;
		mData->RefCounter++;
	}

	RenderResult::~RenderResult()
	{
		PR_ASSERT(mData->RefCounter > 0);
		mData->RefCounter--;
		if (mData->RefCounter == 0)
		{
			delete[] mData->Data;
			delete mData;
			mData = nullptr;
		}
	}

	RenderResult& RenderResult::operator=(const RenderResult& res)
	{
		PR_ASSERT(mData->RefCounter > 0);
		mData->RefCounter--;
		if (mData->RefCounter == 0)
		{
			delete[] mData->Data;
			delete mData;
			mData = nullptr;
		}

		mData = res.mData;
		mData->RefCounter++;

		return *this;
	}

	void RenderResult::clear()
	{
		std::memset(mData->Data, 0, mData->Width*mData->Height*Spectrum::SAMPLING_COUNT * sizeof(float));
	}

	uint32 RenderResult::width() const
	{
		return mData->Width;
	}

	uint32 RenderResult::height() const
	{
		return mData->Height;
	}

	void RenderResult::setPoint(uint32 x, uint32 y, const Spectrum& s)
	{
		const uint32 index = y*mData->Width*Spectrum::SAMPLING_COUNT + x*Spectrum::SAMPLING_COUNT;
		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			mData->Data[index + i] = s.value(i);
		}
	}

	Spectrum RenderResult::point(uint32 x, uint32 y) const
	{
		return Spectrum(&mData->Data[y*mData->Width*Spectrum::SAMPLING_COUNT + x*Spectrum::SAMPLING_COUNT]);
	}

	float* RenderResult::ptr() const
	{
		return mData->Data;
	}
}