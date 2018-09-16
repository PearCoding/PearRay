#include "ILight.h"

#include <sstream>

namespace PR {
ILight::ILight(uint32 id)
	: IFreezable()
	, mID(id)
{
}

std::string ILight::dumpInformation() const
{
	std::stringstream stream;
	stream << std::boolalpha << "<Light> [" << id() << "]: " << std::endl;

	return stream.str();
}
} // namespace PR
