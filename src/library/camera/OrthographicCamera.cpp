#include "OrthographicCamera.h"
#include "ray/Ray.h"

namespace PR
{
	OrthographicCamera::OrthographicCamera(const std::string& name, Entity* parent) :
		Camera(name, parent), mWidth(1), mHeight(1),
		mLensDistance(1)
	{
	}

	OrthographicCamera::~OrthographicCamera()
	{
	}

	std::string OrthographicCamera::type() const
	{
		return "orthographicCamera";
	}

	void OrthographicCamera::setWithAngle(float foh, float fov, float lensdist)
	{
		mWidth = 2 * tan(foh / 2) / lensdist;
		mHeight = 2 * tan(fov / 2) / lensdist;
		mLensDistance = lensdist;
	}

	void OrthographicCamera::setWithSize(float width, float height, float lensdist)
	{
		mWidth = width;
		mHeight = height;
		mLensDistance = lensdist;
	}

	void OrthographicCamera::setWidth(float w)
	{
		mWidth = w;
	}

	float OrthographicCamera::width() const
	{
		return mWidth;
	}

	void OrthographicCamera::setHeight(float h)
	{
		mHeight = h;
	}

	float OrthographicCamera::height() const
	{
		return mHeight;
	}

	void OrthographicCamera::setLensDistance(float f)
	{
		mLensDistance = f;
	}

	float OrthographicCamera::lensDistance() const
	{
		return mLensDistance;
	}

	Ray OrthographicCamera::constructRay(float nx, float ny) const
	{
		float sx = mWidth * nx;
		float sy = mHeight * ny;

		PM::vec3 dir = PM::pm_Set(0, 0, 1, 0);
		return Ray(PM::pm_Multiply(matrix(), PM::pm_Set(sx, sy, 0, 1)),
			PM::pm_Multiply(PM::pm_Rotation(rotation()), dir));
	}
}