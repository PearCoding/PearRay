#pragma once

#include "spectral/Spectrum.h"
#include "PearMath.h"

namespace PR
{
	class PR_LIB Texture1D
	{
	public:
		Texture1D();

		Spectrum eval(float u);

		TextureWrapMode wrapMode() const;
		void setWrapMode(TextureWrapMode mode);

	protected:
		virtual Spectrum getValue(float u) = 0;

	private:
		TextureWrapMode mWrapMode;
	};
}