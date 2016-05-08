#pragma once

#include "Texture2D.h"

namespace PR
{
	class PR_LIB SpecTexture2D : public Texture2D
	{
	public:
		SpecTexture2D(Spectrum* specs, uint32 width, uint32 height);

		uint32 width() const;
		uint32 height() const;

		Spectrum* spectrums() const;

		void setPixel(uint32 px, uint32 py, const Spectrum& spec);
		Spectrum pixel(uint32 px, uint32 py) const;

	protected:
		Spectrum getValue(const PM::vec2& uv) override;

	private:
		Spectrum* mSpectrums;
		uint32 mWidth;
		uint32 mHeight;
	};
}