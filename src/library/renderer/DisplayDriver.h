#pragma once

#include "spectral/Spectrum.h"

namespace PR
{
	class Renderer;
	class PR_LIB_INLINE IDisplayDriver
	{
	public:
		virtual ~IDisplayDriver() {}
		
		virtual void init(Renderer* renderer) = 0;
		virtual void deinit() = 0;

		virtual void clear(uint32 sx = 0, uint32 sy = 0, uint32 ex = 0, uint32 ey = 0) = 0;

		virtual void pushFragment(uint32 x, uint32 y, uint32 layer, const Spectrum& s) = 0;
		virtual Spectrum getFragment(uint32 x, uint32 y, uint32 layer) const = 0;
	};
}