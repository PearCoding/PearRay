#include "Texture1D.h"

namespace PR
{
	Texture1D::Texture1D() :
		mWrapMode(TWM_Clamp)
	{

	}

	Spectrum Texture1D::eval(float u)
	{
		switch (mWrapMode)
		{
		default:
		case TWM_Clamp:
			return getValue(PM::pm_ClampT(u, 0.0f, 1.0f));
		case TWM_Repeat:
			return getValue(u - (int)u);
		case TWM_MirroredRepeat:
		{
			int tx = u;
			float tu = u - tx;
			if (tx % 2 == 1)
				tu = 1 - tu;

			return getValue(tu);
		}
		}
	}

	TextureWrapMode Texture1D::wrapMode() const
	{
		return mWrapMode;
	}

	void Texture1D::setWrapMode(TextureWrapMode mode)
	{
		mWrapMode = mode;
	}
}