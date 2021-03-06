#include "ICamera.h"

namespace PR {

const Vector3f ICamera::DefaultDirection = Vector3f(0, 1, 0);
const Vector3f ICamera::DefaultUp		 = Vector3f(0, 0, 1);
const Vector3f ICamera::DefaultRight	 = Vector3f(1, 0, 0);

ICamera::ICamera(const std::string& name, const Transformf& transform)
	: ITransformable(name, transform)
{
}

ICamera::~ICamera()
{
}
} // namespace PR
