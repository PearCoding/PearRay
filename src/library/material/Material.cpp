#include "Material.h"
#include "ray/Ray.h"

namespace PR
{
	Material::Material() :
		mEmission(nullptr), 
		mIsLight(false), mCanBeShaded(true), mSelfShadow(true), mCameraVisible(true)
	{
	}

	bool Material::isLight() const
	{
		return mIsLight;
	}

	SpectralShaderOutput* Material::emission() const
	{
		return mEmission;
	}

	void Material::setEmission(SpectralShaderOutput* spec)
	{
		mEmission = spec;
		mIsLight = mEmission != nullptr;
	}

	bool Material::canBeShaded() const
	{
		return mCanBeShaded;
	}

	void Material::enableShading(bool b)
	{
		mCanBeShaded = b;
	}

	void Material::enableSelfShadow(bool b)
	{
		mSelfShadow = b;
	}

	bool Material::canBeSelfShadowed() const
	{
		return mSelfShadow;
	}

	void Material::enableCameraVisibility(bool b)
	{
		mCameraVisible = b;
	}

	bool Material::isCameraVisible() const
	{
		return mCameraVisible;
	}
}