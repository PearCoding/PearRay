#include "ConstTexture2D.h"

namespace PR
{
	ConstTexture2D::ConstTexture2D(const Spectrum& spec) :
		Texture2D(), mValue(spec)
	{
	}

	Spectrum ConstTexture2D::value() const
	{
		return mValue;
	}

	void ConstTexture2D::setValue(const Spectrum& spec)
	{
		mValue = spec;
	}

	Spectrum ConstTexture2D::getValue(const PM::vec2& uv)
	{
		return mValue;
	}
}