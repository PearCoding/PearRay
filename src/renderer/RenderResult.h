#pragma once

#include "Config.h"

namespace PR
{
	class Camera;
	class RenderResult
	{
	public:
		RenderResult(uint32 width, uint32 height);
		RenderResult(const RenderResult& res);
		virtual ~RenderResult();

		RenderResult& operator = (const RenderResult& res);

		uint32 width() const;
		uint32 height() const;

		void setPoint(uint32 x, uint32 y, float f);
		float point(uint32 x, uint32 y) const;
	private:
		struct InternalData
		{
			uint32 Width;
			uint32 Height;
			float* Data;
			uint32 RefCounter;
		};

		InternalData* mData;
	};
}