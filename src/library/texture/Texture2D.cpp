#include "Texture2D.h"

namespace PR
{
	Texture2D::Texture2D() :
		mWrapMode(TWM_Clamp)
	{

	}

	Spectrum Texture2D::eval(const PM::vec2& uv)
	{
		switch (mWrapMode)
		{
		default:
		case TWM_Clamp:
			return getValue(PM::pm_Clamp(uv, PM::pm_Set(0, 0), PM::pm_Set(1, 1)));
		case TWM_Repeat:
			return getValue(PM::pm_Set(
				PM::pm_GetX(uv) - (int)PM::pm_GetX(uv),
				PM::pm_GetY(uv) - (int)PM::pm_GetY(uv)));
		case TWM_MirroredRepeat:
		{
			int tx = PM::pm_GetX(uv);
			int ty = PM::pm_GetY(uv);
			float u = PM::pm_GetX(uv) - tx;
			float v = PM::pm_GetY(uv) - tx;
			if (tx % 2 == 1)
				u = 1 - u;
			if (ty % 2 == 1)
				v = 1 - v;

			return getValue(PM::pm_Set(u, v));
		}
		}
	}

	TextureWrapMode Texture2D::wrapMode() const
	{
		return mWrapMode;
	}

	void Texture2D::setWrapMode(TextureWrapMode mode)
	{
		mWrapMode = mode;
	}
}