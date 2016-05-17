#pragma once

#include "spectral/Spectrum.h"
#include "PearMath.h"

namespace PR
{
	template<typename T>
	class PR_LIB_INLINE GenericTexture3D
	{
		PR_CLASS_NON_COPYABLE(GenericTexture3D<T>);
	public:
		inline GenericTexture3D() :
			mWrapMode(TWM_Repeat)
		{
		}

		inline virtual ~GenericTexture3D()
		{
		}

		inline T eval(const PM::vec3& uv)
		{
			switch (mWrapMode)
			{
			default:
			case TWM_Clamp:
				return getValue(PM::pm_Clamp(uv, PM::pm_Set(0, 0, 0), PM::pm_Set(1, 1, 1)));
			case TWM_Repeat:
				return getValue(PM::pm_Clamp(PM::pm_Set(
					PM::pm_GetX(uv) - (int)PM::pm_GetX(uv),
					PM::pm_GetY(uv) - (int)PM::pm_GetY(uv),
					PM::pm_GetZ(uv) - (int)PM::pm_GetZ(uv)),
					PM::pm_Set(0, 0, 0), PM::pm_Set(1, 1, 1)));
			case TWM_MirroredRepeat:
			{
				int tx = (int)PM::pm_GetX(uv);
				int ty = (int)PM::pm_GetY(uv);
				int tz = (int)PM::pm_GetZ(uv);
				float u = PM::pm_GetX(uv) - tx;
				float v = PM::pm_GetY(uv) - ty;
				float w = PM::pm_GetZ(uv) - tz;
				if (tx % 2 == 1)
					u = 1 - u;
				if (ty % 2 == 1)
					v = 1 - v;
				if (tz % 2 == 1)
					w = 1 - w;

				return getValue(PM::pm_Clamp(PM::pm_Set(u, v, w)
					PM::pm_Set(0, 0, 0), PM::pm_Set(1, 1, 1)));
			}
			}
		}

		inline TextureWrapMode wrapMode() const
		{
			return mWrapMode;
		}

		inline void setWrapMode(TextureWrapMode mode)
		{
			mWrapMode = mode;
		}

	protected:
		virtual T getValue(const PM::vec3& uv) = 0;

	private:
		TextureWrapMode mWrapMode;
	};

	typedef GenericTexture3D<float> Data3D;
	typedef GenericTexture3D<Spectrum> Texture3D;
}