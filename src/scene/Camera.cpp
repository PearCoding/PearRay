#include "Camera.h"

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
}