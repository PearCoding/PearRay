#pragma once

#include "spectral/Spectrum.h"

namespace PR
{
	class Renderer;
	class IDisplayDriver;
	class PR_LIB PixelMap
	{
	public:
		PixelMap(Renderer* renderer, IDisplayDriver* driver);
		~PixelMap();
		
		void init();
		void deinit();

		void clear();

		void pushFragment(uint32 x, uint32 y, uint32 layer, const Spectrum& s);
		Spectrum getFragment(uint32 x, uint32 y, uint32 layer) const;

		void setSampleCount(uint32 x, uint32 y, uint32 sample);
		uint32 getSampleCount(uint32 x, uint32 y) const;

		void setPixelError(uint32 x, uint32 y, const Spectrum& pixel, const Spectrum& weight);
		bool isPixelFinished(uint32 x, uint32 y) const;
		uint64 finishedPixelCount() const;
		
	private:
		Renderer* mRenderer;
		IDisplayDriver* mDriver;

		uint32* mSamples;
		float* mPixelError;
	};
}