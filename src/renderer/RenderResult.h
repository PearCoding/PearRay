#pragma once

#include "Config.h"

namespace PR
{
	class Camera;
	class RenderResult
	{
	public:
		RenderResult(Camera* cam);
		RenderResult(const RenderResult& res);
		virtual ~RenderResult();

		RenderResult& operator = (const RenderResult& res);

		void setPoint(uint32 x, uint32 y, float f);
		float point(uint32 x, uint32 y) const;

		void release();
	private:
		Camera* mCamera;
		float* mData;
	};
}