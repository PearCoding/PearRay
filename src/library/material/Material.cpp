#include "Material.h"
#include "ray/Ray.h"

namespace PR
{
	Material::Material() :
		mEmission(), 
		mCanBeShaded(true), mSelfShadow(true), mCameraVisible(true), mIsLight(false)
	{
	}

	bool Material::isLight() const
	{
		return mIsLight;
	}

	Spectrum Material::emission() const
	{
		return mEmission;
	}

	void Material::setEmission(const Spectrum& spec)
	{
		mEmission = spec;
		mIsLight = !mEmission.isOnlyZero();
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

	bool Material::shouldIgnore_Simple(const Ray& in, RenderEntity* entity)
	{
		return !mCameraVisible && in.depth() == 0;
	}
	
	Spectrum Material::applyEmission(const FacePoint& point, const PM::vec3& V)
	{
		return mEmission;
	}
}