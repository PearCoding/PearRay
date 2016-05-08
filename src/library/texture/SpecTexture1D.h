#pragma once

#include "Texture1D.h"

namespace PR
{
	class PR_LIB SpecTexture1D : public Texture1D
	{
	public:
		SpecTexture1D(Spectrum* specs, uint32 width);

		uint32 width() const;

		Spectrum* spectrums() const;

		void setPixel(uint32 px, const Spectrum& spec);
		Spectrum pixel(uint32 px) const;

	protected:
		Spectrum getValue(float u) override;

	private:
		Spectrum* mSpectrums;
		uint32 mWidth;
	};
}