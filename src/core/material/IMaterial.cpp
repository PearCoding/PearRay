#include "IMaterial.h"

#include <sstream>

namespace PR {
IMaterial::IMaterial()
	: mCanBeShaded(true)
	, mShadow(true)
	, mSelfShadow(true)
	, mCameraVisible(true)
{
}

std::string IMaterial::dumpInformation() const
{
	std::stringstream stream;
	stream << std::boolalpha << "<Material>: " << std::endl
		   << "  IsShadeable:       " << canBeShaded() << std::endl
		   << "  IsCameraVisible:   " << isCameraVisible() << std::endl
		   << "  SelfShadowing:     " << allowsSelfShadow() << std::endl
		   << "  Shadows:           " << allowsShadow() << std::endl;

	return stream.str();
}
} // namespace PR
