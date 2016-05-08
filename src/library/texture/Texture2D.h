#pragma once

#include "spectral/Spectrum.h"
#include "PearMath.h"

namespace PR
{
	class PR_LIB Texture2D
	{
	public:
		Texture2D();

		Spectrum eval(const PM::vec2& uv);

		TextureWrapMode wrapMode() const;
		void setWrapMode(TextureWrapMode mode);

	protected:
		virtual Spectrum getValue(const PM::vec2& uv) = 0;

	private:
		TextureWrapMode mWrapMode;
	};
}