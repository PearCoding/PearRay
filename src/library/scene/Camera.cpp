#include "Camera.h"
#include "ray/Ray.h"

namespace PR
{
	Camera::Camera(const std::string& name, Entity* parent) :
		Entity(name, parent), mWidth(1), mHeight(1), mLensDistance(0.1f)
	{
		cache();
	}

	Camera::~Camera()
	{
	}

	std::string Camera::type() const
	{
		return "camera";
	}

	void Camera::setWithAngle(float foh, float fov, float lensdist)
	{
		mWidth = 2 * sin(foh / 2) / lensdist;
		mHeight = 2 * sin(fov / 2) / lensdist;
		mLensDistance = lensdist;
		cache();
	}

	void Camera::setWithSize(float width, float height, float lensdist)
	{
		mWidth = width;
		mHeight = height;
		mLensDistance = lensdist;
		cache();
	}

	void Camera::setWidth(float w)
	{
		mWidth = w;
		cache();
	}

	float Camera::width() const
	{
		return mWidth;
	}

	void Camera::setHeight(float h)
	{
		mHeight = h;
		cache();
	}

	float Camera::height() const
	{
		return mHeight;
	}

	void Camera::setLensDistance(float f)
	{
		mLensDistance = f;
		cache();
	}

	float Camera::lensDistance() const
	{
		return mLensDistance;
	}

	Ray Camera::constructRay(float sx, float sy) const
	{
		PM::vec3 dir = PM::pm_Normalize3D(PM::pm_Set(sx, sy, 1));

		return Ray(PM::pm_Multiply(mMatrix, PM::pm_Multiply(matrix(), PM::pm_Set(sx, sy, 0, 1))),
			PM::pm_Multiply(PM::pm_Rotation(rotation()), dir));
	}

	void Camera::cache()
	{
		mMatrix = PM::pm_Perspective(width(), height(), lensDistance(), 100);// TODO
	}
}