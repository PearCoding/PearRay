#include "IMaterial.h"

#include <sstream>

namespace PR {
IMaterial::IMaterial(uint32 id)
	: mCanBeShaded(true)
	, mShadow(true)
	, mSelfShadow(true)
	, mCameraVisible(true)
	, mID(id)
{
}

std::string IMaterial::dumpInformation() const
{
	int flags = this->flags();
	std::stringstream stream;
	stream << std::boolalpha << "<Material>: " << std::endl
		   << "  IsShadeable:       " << canBeShaded() << std::endl
		   << "  IsCameraVisible:   " << isCameraVisible() << std::endl
		   << "  SelfShadowing:     " << allowsSelfShadow() << std::endl
		   << "  Shadows:           " << allowsShadow() << std::endl
		   << "  SpectralVarying:   " << ((flags & MF_SpectralVarying) != 0) << std::endl
		   << "  DeltaDistribution: " << ((flags & MF_DeltaDistribution) != 0) << std::endl;

	return stream.str();
}
} // namespace PR
