#include "Texture2D.h"

namespace PR
{
	Texture2D::Texture2D() :
		mWrapMode(TWM_Clamp)
	{

	}

	Spectrum Texture2D::eval(const PM::vec2& uv)

	TextureWrapMode Texture2D::wrapMode() const

	void Texture2D::setWrapMode(TextureWrapMode mode)
}