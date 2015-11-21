#include "Camera.h"
#include "ray/Ray.h"

namespace PR
{
	Camera::Camera(const std::string& name, Entity* parent) :
		Entity(name, parent)
	{
	}

	Camera::~Camera()
	{
	}
}