#include "Material.h"
#include "ray/Ray.h"

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
