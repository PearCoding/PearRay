#include "Texture3D.h"

namespace PR
{
	Texture3D::Texture3D() :
		mWrapMode(TWM_Clamp)
	{

	}

	Spectrum Texture3D::eval(const PM::vec3& uv)
	

	TextureWrapMode Texture3D::wrapMode() const
	{
		return mWrapMode;
	}

	void Texture3D::setWrapMode(TextureWrapMode mode)
	{
		mWrapMode = mode;
	}
}