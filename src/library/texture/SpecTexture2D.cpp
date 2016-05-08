#include "SpecTexture2D.h"

namespace PR
{
	SpecTexture2D::SpecTexture2D(Spectrum* specs, uint32 width, uint32 height) :
		Texture2D(), mSpectrums(specs), mWidth(width), mHeight(height)
	{
		PR_ASSERT(specs);
		PR_ASSERT(width > 0);
		PR_ASSERT(height > 0);
	}

	SpecTexture2D::~SpecTexture2D()
	{
		// NO REFERENCE COUNTING!
		delete[] mSpectrums;
	}

	uint32 SpecTexture2D::width() const
	{
		return mWidth;
	}

	uint32 SpecTexture2D::height() const
	{
		return mHeight;
	}

	Spectrum* SpecTexture2D::spectrums() const
	{
		return mSpectrums;
	}

	void SpecTexture2D::setPixel(uint32 px, uint32 py, const Spectrum& spec)
	{
		mSpectrums[py * mWidth + px] = spec;
	}

	Spectrum SpecTexture2D::pixel(uint32 px, uint32 py) const
	{
		return mSpectrums[py * mWidth + px];
	}

	Spectrum SpecTexture2D::getValue(const PM::vec2& uv)
	{
		// TODO: Use interpolation!
		uint32 px = PM::pm_GetX(uv) * mWidth;
		uint32 py = PM::pm_GetY(uv) * mHeight;
		return pixel(px, py);
	}
}