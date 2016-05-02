#include "OrthographicCamera.h"
#include "ray/Ray.h"

namespace PR
{
	OrthographicCamera::OrthographicCamera(const std::string& name, Entity* parent) :
		Camera(name, parent), mWidth(1), mHeight(1),
		mLensDistance(1), mLookAt(PM::pm_Set(0,0,0,1))
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

	void OrthographicCamera::lookAt(const PM::vec3& v)
	{
		mLookAt = v;
	}

	PM::vec3 OrthographicCamera::lookAtPosition() const
	{
		return mLookAt;
	}

	Ray OrthographicCamera::constructRay(float nx, float ny) const
	{
		float sx = - scale() * mWidth * nx;
		float sy = - scale() * mHeight * ny;

		return Ray(PM::pm_Add(position(), PM::pm_Add(PM::pm_Scale(mRight_Cache, sx), PM::pm_Scale(mUp_Cache, sy))),
			mDirection_Cache);
	}

	// Cache
	void OrthographicCamera::onPreRender()
	{
		mDirection_Cache = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Subtract(mLookAt, position())), 0);

		if (PM::pm_MagnitudeSqr3D(mDirection_Cache) < PM_EPSILON)
			mDirection_Cache = PM::pm_Set(0, 0, 1);

		float dot = PM::pm_Dot3D(mDirection_Cache, PM::pm_Set(0, 1, 0));

		if (dot >= 1)
			mRight_Cache = PM::pm_Set(1, 0, 0);
		else if (dot <= -1)
			mRight_Cache = PM::pm_Set(-1, 0, 0);
		else
			mRight_Cache = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Cross3D(mDirection_Cache, PM::pm_Set(0, 1, 0))), 0);

		mUp_Cache = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Cross3D(mRight_Cache, mDirection_Cache)), 0);
	}
}