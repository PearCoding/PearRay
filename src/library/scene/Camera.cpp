#include "Camera.h"
#include "ray/Ray.h"

namespace PR
{
	Camera::Camera(const std::string& name, Entity* parent) :
		Entity(name, parent), mWidth(1), mHeight(1),
		mLensDistance(1), mOrthographic(false)
	{
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
	}

	void Camera::setWithSize(float width, float height, float lensdist)
	{
		mWidth = width;
		mHeight = height;
		mLensDistance = lensdist;
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

	void Camera::setOrthographic(bool b)
	{
		mOrthographic = b;
	}

	bool Camera::isOrthographic() const
	{
		return mOrthographic;
	}

	Ray Camera::constructRay(float sx, float sy) const
	{
		if (isOrthographic())
		{
			PM::vec3 dir = PM::pm_Set(0, 0, 1);

			return Ray(PM::pm_Multiply(matrix(), PM::pm_Set(sx, sy, 0, 1)),
				PM::pm_Multiply(PM::pm_Rotation(rotation()), dir));
		}
		else// Doesn't work
		{
			PM::vec3 dir = PM::pm_Normalize3D(PM::pm_Set(sx, sy, lensDistance()));

			return Ray(PM::pm_Multiply(matrix(), PM::pm_Set(0,0,0,1)),
				PM::pm_Multiply(PM::pm_Rotation(rotation()), dir));
		}
	}
}