#include "Camera.h"
#include "ray/Ray.h"

namespace PR
{
	Camera::Camera(float width, float height, float lens, const std::string& name, Entity* parent) :
		Entity(name, parent), mWidth(width), mHeight(height), mLensDistance(lens)
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::setWidth(float w)
	{
		mWidth = w;
	}

	float Camera::width() const
	{
		return mWidth;
	}

	void Camera::setHeight(float h)
	{
		mHeight = h;
	}

	float Camera::height() const
	{
		return mHeight;
	}

	void Camera::setLensDistance(float f)
	{
		mLensDistance = f;
	}

	float Camera::lensDistance() const
	{
		return mLensDistance;
	}

	Ray Camera::constructRay(float sx, float sy) const
	{
		PM::vec3 dir = PM::pm_Normalize3D(PM::pm_Set(sx, sy, lensDistance()));

		return Ray(PM::pm_Multiply(matrix(), PM::pm_Set(sx, sy, 0, 1)),
			PM::pm_Multiply(PM::pm_Rotation(rotation()), dir));
	}
}