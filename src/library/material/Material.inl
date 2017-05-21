#pragma once

namespace PR
{
	inline uint32 Material::id() const
	{
		return mID;
	}

	inline bool Material::isLight() const
	{
		return mEmission != nullptr;
	}

	inline const std::shared_ptr<SpectralShaderOutput>& Material::emission() const
	{
		return mEmission;
	}

	inline void Material::setEmission(const std::shared_ptr<SpectralShaderOutput>& spec)
	{
		mEmission = spec;
	}

	inline bool Material::canBeShaded() const
	{
		return mCanBeShaded;
	}

	inline void Material::enableShading(bool b)
	{
		mCanBeShaded = b;
	}

	inline void Material::enableShadow(bool b)
	{
		mShadow = b;
	}

	inline bool Material::allowsShadow() const
	{
		return mShadow;
	}

	inline void Material::enableSelfShadow(bool b)
	{
		mSelfShadow = b;
	}

	inline bool Material::allowsSelfShadow() const
	{
		return mSelfShadow;
	}

	inline void Material::enableCameraVisibility(bool b)
	{
		mCameraVisible = b;
	}

	inline bool Material::isCameraVisible() const
	{
		return mCameraVisible;
	}
}
