#include "RGBTexture2D.h"
#include "spectral/RGBConverter.h"

namespace PR
{
	RGBTexture2D::RGBTexture2D(float* rgba, uint32 width, uint32 height) :
		Texture2D(), mRGBA(rgba), mWidth(width), mHeight(height)
	{
		PR_ASSERT(rgba);
		PR_ASSERT(width > 0);
		PR_ASSERT(height > 0);

		// To linear
		// TODO: Should be optional
		for (uint32 i = 0; i < width; ++i)
		{
			for (uint32 j = 0; j < height; ++j)
			{
				for (uint32 e = 0; e < 3; ++e)
				{
					mRGBA[j * width * 4 + i * 4 + e] = std::pow(mRGBA[j * width * 4 + i * 4 + e], 2.2f);
				}
			}
		}
	}

	RGBTexture2D::~RGBTexture2D()
	{
		// NO REFERENCE COUNTING!
		delete[] mRGBA;
	}

	uint32 RGBTexture2D::width() const
	{
		return mWidth;
	}

	uint32 RGBTexture2D::height() const
	{
		return mHeight;
	}

	float* RGBTexture2D::pixels() const
	{
		return mRGBA;
	}

	void RGBTexture2D::setPixel(uint32 px, uint32 py, const PM::color& color)
	{
		float* ptr = &mRGBA[py * 4 * mWidth + px * 4];
		ptr[0] = PM::pm_GetR(color);
		ptr[1] = PM::pm_GetG(color);
		ptr[2] = PM::pm_GetB(color);
		ptr[3] = PM::pm_GetA(color);
	}

	PM::color RGBTexture2D::pixel(uint32 px, uint32 py) const
	{
		const float* ptr = &mRGBA[py * 4 * mWidth + px * 4];
		return PM::pm_Set(ptr[0], ptr[1], ptr[2], ptr[3]);
	}

	Spectrum RGBTexture2D::getValue(const PM::vec2& uv)
	{
		// TODO: Use interpolation!
		uint32 px = PM::pm_GetX(uv) * mWidth;
		uint32 py = PM::pm_GetY(uv) * mHeight;

		auto color = pixel(px, py);
		return RGBConverter::toSpec(PM::pm_GetR(color), PM::pm_GetG(color), PM::pm_GetB(color));
	}
}