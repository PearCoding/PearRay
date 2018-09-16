#include "ICamera.h"

namespace PR {
ICamera::ICamera(uint32 id, const std::string& name)
	: VirtualEntity(id, name)
{
}

ICamera::~ICamera()
{
}
}
