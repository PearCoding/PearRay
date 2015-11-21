#include "PerspectiveCamera.h"
#include "ray/Ray.h"

namespace PR
{
	PerspectiveCamera::PerspectiveCamera(const std::string& name, Entity* parent) :
		Camera(name, parent), mWidth(1), mHeight(1),
		mLensDistance(1)
	{
	}

	PerspectiveCamera::~PerspectiveCamera()
	{
	}

	std::string PerspectiveCamera::type() const
	{
		return "perspectiveCamera";
	}

	void PerspectiveCamera::setWithAngle(float foh, float fov, float lensdist)
	{
		mWidth = 2 * sin(foh / 2) / lensdist;
		mHeight = 2 * sin(fov / 2) / lensdist;
		mLensDistance = lensdist;
	}

	void PerspectiveCamera::setWithSize(float width, float height, float lensdist)
	{
		mWidth = width;
		mHeight = height;
		mLensDistance = lensdist;
	}

	void PerspectiveCamera::setWidth(float w)
	{
		mWidth = w;
	}

	float PerspectiveCamera::width() const
	{
		return mWidth;
	}

	void PerspectiveCamera::setHeight(float h)
	{
		mHeight = h;
	}

	float PerspectiveCamera::height() const
	{
		return mHeight;
	}

	void PerspectiveCamera::setLensDistance(float f)
	{
		mLensDistance = f;
	}

	float PerspectiveCamera::lensDistance() const
	{
		return mLensDistance;
	}

	Ray PerspectiveCamera::constructRay(float nx, float ny) const
	{
		float sx = mWidth * (nx - 0.5f);
		float sy = mHeight * (ny - 0.5f);

		PM::vec3 dir = PM::pm_Normalize3D(PM::pm_Set(sx, sy, lensDistance()));

		return Ray(PM::pm_Multiply(matrix(), PM::pm_Set(0,0,0,1)),
				PM::pm_Multiply(PM::pm_Rotation(rotation()), dir));
	}
}