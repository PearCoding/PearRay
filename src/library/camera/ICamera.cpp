#include "ICamera.h"

namespace PR {
ICamera::ICamera(uint32 id, const std::string& name)
	: ITransformable(id, name)
{
}

ICamera::~ICamera()
{
}
}
