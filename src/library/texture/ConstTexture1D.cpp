#include "ConstTexture1D.h"

namespace PR
{
	ConstTexture1D::ConstTexture1D(const Spectrum& spec) :
		Texture1D(), mValue(spec)
	{
	}

	Spectrum ConstTexture1D::value() const
	{
		return mValue;
	}

	void ConstTexture1D::setValue(const Spectrum& spec)
	{
		mValue = spec;
	}

	Spectrum ConstTexture1D::getValue(float u)
	{
		return mValue;
	}
}