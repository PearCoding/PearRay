#include "Texture3D.h"

namespace PR
{
	Texture3D::Texture3D() :
		mWrapMode(TWM_Clamp)
	{

	}

	Spectrum Texture3D::eval(const PM::vec3& uv)
	{
		switch (mWrapMode)
		{
		default:
		case TWM_Clamp:
			return getValue(PM::pm_Clamp(uv, PM::pm_Set(0, 0, 0), PM::pm_Set(1, 1, 1)));
		case TWM_Repeat:
			return getValue(PM::pm_Set(
				PM::pm_GetX(uv) - (int)PM::pm_GetX(uv),
				PM::pm_GetY(uv) - (int)PM::pm_GetY(uv),
				PM::pm_GetZ(uv) - (int)PM::pm_GetZ(uv)));
		case TWM_MirroredRepeat:
		{
			int tx = PM::pm_GetX(uv);
			int ty = PM::pm_GetY(uv);
			int tz = PM::pm_GetZ(uv);
			float u = PM::pm_GetX(uv) - tx;
			float v = PM::pm_GetY(uv) - ty;
			float w = PM::pm_GetZ(uv) - tz;
			if (tx % 2 == 1)
				u = 1 - u;
			if (ty % 2 == 1)
				v = 1 - v;
			if (tz % 2 == 1)
				w = 1 - w;

			return getValue(PM::pm_Set(u, v));
		}
		}
	}

	TextureWrapMode Texture3D::wrapMode() const
	{
		return mWrapMode;
	}

	void Texture3D::setWrapMode(TextureWrapMode mode)
	{
		mWrapMode = mode;
	}
}