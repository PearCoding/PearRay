#include "Camera.h"

namespace PR
{
	Camera::Camera(uint32 width, uint32 height, float vangle, float hangle) :
		mWidth(width), mHeight(height), mVAngle(vangle), mHAngle(hangle)
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::setWidth(uint32 w)
	{
		mWidth = w;
	}

	uint32 Camera::width() const
	{
		return mWidth;
	}

	void Camera::setHeight(uint32 h)
	{
		mHeight = h;
	}

	uint32 Camera::height() const
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