#include "Serializer.h"
#include "Version.h"

namespace PR {
Serializer::Serializer(bool readmode, uint32 version)
	: mReadMode(readmode)
	, mVersion(version == 0 ? PR_VERSION : version)
{
}

Serializer::~Serializer()
{
}

} // namespace PR