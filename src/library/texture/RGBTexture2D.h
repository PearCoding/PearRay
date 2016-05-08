#pragma once

#include "Texture2D.h"

namespace PR
{
	class PR_LIB RGBTexture2D : public Texture2D
	{
	public:
		RGBTexture2D(float* rgba, uint32 width, uint32 height);
		~RGBTexture2D();

		uint32 width() const;
		uint32 height() const;

		// Always RGBA!
		float* pixels() const;

		void setPixel(uint32 px, uint32 py, const PM::color& color);
		PM::color pixel(uint32 px, uint32 py) const;

	protected:
		Spectrum getValue(const PM::vec2& uv) override;

	private:
		float* mRGBA;
		uint32 mWidth;
		uint32 mHeight;
	};
}