#pragma once

#include "spectral/Spectrum.h"
#include "PearMath.h"

namespace PR
{
	class PR_LIB Texture3D
	{
	public:
		Texture3D();

		Spectrum eval(const PM::vec3& uv);

		TextureWrapMode wrapMode() const;
		void setWrapMode(TextureWrapMode mode);

	protected:
		virtual Spectrum getValue(const PM::vec3& uv) = 0;

	private:
		TextureWrapMode mWrapMode;
	};
}