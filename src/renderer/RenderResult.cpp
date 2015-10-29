#include "RenderResult.h"
#include "scene/Camera.h"

namespace PR
{
	RenderResult::RenderResult(Camera* cam) :
		mCamera(cam), mData(nullptr)
	{
		PR_ASSERT(cam);
		mData = new float[cam->width() * cam->height()];
		memset(mData, 0, cam->width() * cam->height() * sizeof(float));
	}

	RenderResult::RenderResult(const RenderResult& res)
	{
		mCamera = res.mCamera;
		mData = res.mData;
	}

	RenderResult::~RenderResult()
	{
	}

	RenderResult& RenderResult::operator=(const RenderResult& res)
	{
		mCamera = res.mCamera;
		mData = res.mData;

		return *this;
	}

	void RenderResult::setPoint(uint32 x, uint32 y, float f)
	{
		mData[y*mCamera->width() + x] = f;
	}

	float RenderResult::point(uint32 x, uint32 y) const
	{
		return mData[y*mCamera->width() + x];
	}

	void RenderResult::release()
	{
		PR_ASSERT(mData);
		delete[] mData;
		mData = nullptr;
	}
}