#pragma once

#include "spectral/Spectrum.h"
#include "PearMath.h"

namespace PR
{
	template<typename T>
	class PR_LIB_INLINE GenericTexture1D
	{
		PR_CLASS_NON_COPYABLE(GenericTexture1D<T>);
	public:
		inline GenericTexture1D() :
			mWrapMode(TWM_Clamp)
		{
		}

		inline virtual ~GenericTexture1D()
		{
		}

		inline T eval(float u)
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
				int tx = (int)u;
				float tu = u - tx;
				if (tx % 2 == 1)
					tu = 1 - tu;

				return getValue(tu);
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
		virtual T getValue(float u) = 0;

	private:
		TextureWrapMode mWrapMode;
	};

	typedef GenericTexture1D<float> Data1D;
	typedef GenericTexture1D<Spectrum> Texture1D;
}