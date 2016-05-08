#include "SpecTexture1D.h"

namespace PR
{
	SpecTexture1D::SpecTexture1D(Spectrum* specs, uint32 width) :
		Texture1D(), mSpectrums(specs), mWidth(width)
	{
		PR_ASSERT(specs);
		PR_ASSERT(width > 0);
	}

	uint32 SpecTexture1D::width() const
	{
		return mWidth;
	}

	Spectrum* SpecTexture1D::spectrums() const
	{
		return mSpectrums;
	}

	void SpecTexture1D::setPixel(uint32 px, const Spectrum& spec)
	{
		mSpectrums[px] = spec;
	}

	Spectrum SpecTexture1D::pixel(uint32 px) const
	{
		return mSpectrums[px];
	}

	Spectrum SpecTexture1D::getValue(float u)
	{
		// TODO: Use interpolation!
		uint32 px = u * mWidth;
		return pixel(px);
	}
}