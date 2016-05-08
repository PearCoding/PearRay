#pragma once

#include "Texture1D.h"

namespace PR
{
	class PR_LIB RGBTexture1D : public Texture1D
	{
	public:
		RGBTexture1D(float* rgba, uint32 width);
		~RGBTexture1D();

		uint32 width() const;

		float* pixels() const;

		void setPixel(uint32 px, const PM::color& color);
		PM::color pixel(uint32 px) const;

	protected:
		Spectrum getValue(float u) override;

	private:
		float* mRGBA;
		uint32 mWidth;
	};
}