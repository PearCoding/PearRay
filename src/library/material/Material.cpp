#include "Material.h"
#include "ray/Ray.h"

namespace PR
{
	Material::Material(uint32 id) :
		mEmission(nullptr), mID(id),
		mIsLight(false), mCanBeShaded(true), mShadow(true), mSelfShadow(true), mCameraVisible(true)
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
	
	void Material::enableShadow(bool b)
	{
		mShadow = b;
	}

	bool Material::allowsShadow() const
	{
		return mShadow;
	}

	void Material::enableSelfShadow(bool b)
	{
		mSelfShadow = b;
	}

	bool Material::allowsSelfShadow() const
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