#include "RenderResult.h"
#include "scene/Camera.h"

namespace PR
{
	RenderResult::RenderResult(uint32 w, uint32 h) :
		mData(nullptr)
	{
		mData = new InternalData;
		mData->Width = w;
		mData->Height = h;
		mData->Data = new float[mData->Width*mData->Height];
		mData->RefCounter = 1;

		memset(mData->Data, 0, mData->Width * mData->Height * sizeof(float));
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

	uint32 RenderResult::width() const
	{
		return mData->Width;
	}

	uint32 RenderResult::height() const
	{
		return mData->Height;
	}

	void RenderResult::setPoint(uint32 x, uint32 y, float f)
	{
		mData->Data[y*mData->Width + x] = f;
	}

	float RenderResult::point(uint32 x, uint32 y) const
	{
		return mData->Data[y*mData->Width + x];
	}
}