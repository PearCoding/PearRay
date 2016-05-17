#pragma once

#include "spectral/Spectrum.h"
#include "PearMath.h"

namespace PR
{
	template<typename T>
	class PR_LIB_INLINE GenericTexture2D
	{
		PR_CLASS_NON_COPYABLE(GenericTexture2D<T>);
	public:
		inline GenericTexture2D() :
			mWrapMode(TWM_Repeat)
		{
		}

		inline virtual ~GenericTexture2D()
		{
		}

		inline T eval(const PM::vec2& uv)
		{
			float tmp;
			switch (mWrapMode)
			{
			default:
			case TWM_Clamp:
				return getValue(PM::pm_Clamp(uv, PM::pm_Set(0, 0), PM::pm_Set(1, 1)));
			case TWM_Repeat:
				return getValue(PM::pm_Set(
					std::abs(std::modf(PM::pm_GetX(uv), &tmp)),
					std::abs(std::modf(PM::pm_GetY(uv), &tmp))));
			case TWM_MirroredRepeat:
			{
				int tx = (int)PM::pm_GetX(uv);
				int ty = (int)PM::pm_GetY(uv);
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

		inline TextureWrapMode wrapMode() const
		{
			return mWrapMode;
		}

		inline void setWrapMode(TextureWrapMode mode)
		{
			mWrapMode = mode;
		}

	protected:
		virtual T getValue(const PM::vec2& uv) = 0;

	private:
		TextureWrapMode mWrapMode;
	};

	typedef GenericTexture2D<float> Data2D;
	typedef GenericTexture2D<Spectrum> Texture2D;
}