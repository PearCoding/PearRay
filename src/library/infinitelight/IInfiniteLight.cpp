#include "IInfiniteLight.h"

#include <sstream>

namespace PR {
IInfiniteLight::IInfiniteLight(uint32 id, const std::string& name)
	: ITransformable(id, name)
{
}

std::string IInfiniteLight::dumpInformation() const
{
	std::stringstream stream;
	stream << std::boolalpha
		   << ITransformable::dumpInformation()
		   << "  <InfiniteLight>: " << std::endl;

	return stream.str();
}
} // namespace PR
