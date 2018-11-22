#include "IEmission.h"

#include <sstream>

namespace PR {
IEmission::IEmission(uint32 id)
	: IFreezable()
	, mID(id)
{
}

std::string IEmission::dumpInformation() const
{
	std::stringstream stream;
	stream << std::boolalpha << "<Light> [" << id() << "]: " << std::endl;

	return stream.str();
}
} // namespace PR
