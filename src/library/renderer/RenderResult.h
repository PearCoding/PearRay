#pragma once

#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB RenderResult
	{
	public:
		RenderResult(uint32 width, uint32 height);
		RenderResult(const RenderResult& res);
		virtual ~RenderResult();

		RenderResult& operator = (const RenderResult& res);

		uint32 width() const;
		uint32 height() const;

		void setPoint(uint32 x, uint32 y, const Spectrum& s);
		Spectrum point(uint32 x, uint32 y) const;

		void clear();

	private:
		struct InternalData
		{
			uint32 Width;
			uint32 Height;
			Spectrum* Data;
			uint32 RefCounter;
		};

		InternalData* mData;
	};
}