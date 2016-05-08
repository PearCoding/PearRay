#include "RGBTexture1D.h"
#include "spectral/RGBConverter.h"

namespace PR
{
	RGBTexture1D::RGBTexture1D(float* rgba, uint32 width) :
		Texture1D(), mRGBA(rgba), mWidth(width)
	{
		PR_ASSERT(rgba);
		PR_ASSERT(width > 0);
	}

	uint32 RGBTexture1D::width() const
	{
		return mWidth;
	}

	float* RGBTexture1D::pixels() const
	{
		return mRGBA;
	}

	void RGBTexture1D::setPixel(uint32 px, const PM::color& color)
	{
		float* ptr = &mRGBA[px * 4];
		ptr[0] = PM::pm_GetR(color);
		ptr[1] = PM::pm_GetG(color);
		ptr[2] = PM::pm_GetB(color);
		ptr[3] = PM::pm_GetA(color);
	}

	PM::color RGBTexture1D::pixel(uint32 px) const
	{
		const float* ptr = &mRGBA[px * 4];
		return PM::pm_Set(ptr[0], ptr[1], ptr[2], ptr[3]);
	}

	Spectrum RGBTexture1D::getValue(float u)
	{
		// TODO: Use interpolation!
		uint32 px = u * mWidth;

		auto color = pixel(px);
		return RGBConverter::toSpec(PM::pm_GetR(color), PM::pm_GetG(color), PM::pm_GetB(color));
	}
}