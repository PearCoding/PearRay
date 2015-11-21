#include "RenderResult.h"

namespace PR
{
	RenderResult::RenderResult(uint32 w, uint32 h) :
		mData(nullptr)
	{
		mData = new InternalData;
		mData->Width = w;
		mData->Height = h;
		mData->Data = new Spectrum[mData->Width*mData->Height];
		mData->Depth = new float[mData->Width*mData->Height];
		mData->RefCounter = 1;

		for (size_t i = 0; i < mData->Width*mData->Height; ++i)
		{
			mData->Depth[i] = -1;
		}
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
			delete[] mData->Depth;
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
			delete[] mData->Depth;
			delete mData;
			mData = nullptr;
		}

		mData = res.mData;
		mData->RefCounter++;

		return *this;
	}

	void RenderResult::clear()
	{
		for (size_t i = 0; i < mData->Width*mData->Height; ++i)
		{
			mData->Data[i] = Spectrum();
			mData->Depth[i] = -1;
		}
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
		mData->Data[y*mData->Width + x] = s;
	}

	Spectrum RenderResult::point(uint32 x, uint32 y) const
	{
		return mData->Data[y*mData->Width + x];
	}

	void RenderResult::setDepth(uint32 x, uint32 y, float f)
	{
		mData->Depth[y*mData->Width + x] = f;
	}

	float RenderResult::depth(uint32 x, uint32 y) const
	{
		return mData->Depth[y*mData->Width + x];
	}

	float RenderResult::maxDepth() const
	{
		float d = 0;
		for (size_t i = 0; i < mData->Width * mData->Height; ++i)
		{
			if (d < mData->Depth[i])
			{
				d = mData->Depth[i];
			}
		}

		return d;
	}
}