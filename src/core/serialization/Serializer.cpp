#include "Serializer.h"
#include "config/Build.h"

namespace PR {
Serializer::Serializer(bool readmode, uint32 version)
	: mReadMode(readmode)
	, mVersion(version == 0 ? Build::getVersion().asNumber() : version)
{
}

Serializer::~Serializer()
{
}

} // namespace PR