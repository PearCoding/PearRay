#include "Camera.h"

namespace PR
{
	Camera::Camera(float width, float height, float vangle, float hangle, const std::string& name, Entity* parent) :
		Entity(name, parent), mWidth(width), mHeight(height), mVAngle(vangle), mHAngle(hangle)
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

	void Camera::setVerticalAngle(float angle)
	{
		mVAngle = angle;
	}

	float Camera::verticalAngle() const
	{
		return mVAngle;
	}

	void Camera::setHorizontalAngle(float angle)
	{
		mHAngle = angle;
	}

	float Camera::horizontalAngle() const
	{
		return mHAngle;
	}
}