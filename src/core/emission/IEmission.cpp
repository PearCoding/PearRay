#include "IEmission.h"

#include <sstream>

namespace PR {
std::string IEmission::dumpInformation() const
{
	std::stringstream stream;
	stream << std::boolalpha << "<Emission>: " << std::endl;

	return stream.str();
}
} // namespace PR
