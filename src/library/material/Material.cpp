#include "Material.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

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

	Texture2D* Material::emission() const
	{
		return mEmission;
	}

	void Material::setEmission(Texture2D* spec)
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

	bool Material::shouldIgnore(const Ray& in, const FacePoint& point)
	{
		return !mCameraVisible && in.depth() == 0;
	}
	
	Spectrum Material::applyEmission(const FacePoint& point, const PM::vec3& V)
	{
		if (mEmission)
			return mEmission->eval(point.uv());
		else
			return Spectrum();
	}
}