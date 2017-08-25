#include "Material.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"

#include <sstream>

namespace PR {
Material::Material(uint32 id)
	: mEmission(nullptr)
	, mID(id)
	, mCanBeShaded(true)
	, mShadow(true)
	, mSelfShadow(true)
	, mCameraVisible(true)
{
}

Spectrum Material::evalEmission(const ShaderClosure& point)
{
	if(!mEmission)
		return Spectrum();
	
	const float NdotV = std::max(0.0f, -point.Ng.dot(point.V));
	return mEmission->eval(point) * NdotV;
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
