#include "Material.h"
#include "ray/Ray.h"

#include <sstream>

namespace PR
{
	Material::Material(uint32 id) :
		mEmission(nullptr), mID(id),
		mCanBeShaded(true), mShadow(true), mSelfShadow(true), mCameraVisible(true)
	{
	}

	SpectralShaderOutput* Material::emission() const
	{
		return mEmission;
	}

	void Material::setEmission(SpectralShaderOutput* spec)
	{
		mEmission = spec;
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

	std::string Material::dumpInformation() const
	{
		std::stringstream stream;
		stream << std::boolalpha << "<Material> [" << mID << "]: " << std::endl
			<< "  IsLight:         " << isLight() << std::endl
			<< "  IsShadeable:     " << canBeShaded() << std::endl
			<< "  IsCameraVisible: " << isCameraVisible() << std::endl
			<< "  SelfShadowing:   " << allowsSelfShadow() << std::endl
			<< "  Shadows:         " << allowsShadow() << std::endl
			<< "  PathCount:       " << samplePathCount() << std::endl;

		return stream.str();
	}
}
