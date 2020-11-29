#include "IInfiniteLight.h"

#include <sstream>

namespace PR {
IInfiniteLight::IInfiniteLight(const std::string& name, const Transformf& transform)
	: ITransformable(name, transform)
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
