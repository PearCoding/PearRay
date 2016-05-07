#include "PerspectiveCamera.h"
#include "ray/Ray.h"

namespace PR
{
	PerspectiveCamera::PerspectiveCamera(const std::string& name, Entity* parent) :
		Camera(name, parent), mWidth(1), mHeight(1),
		mLensDistance(1), mLookAt(PM::pm_Set(0,0,1,1))
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
		mWidth = 2 * tan(foh / 2) / lensdist;
		mHeight = 2 * tan(fov / 2) / lensdist;
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

	void PerspectiveCamera::lookAt(const PM::vec3& v)
	{
		mLookAt = v;
	}

	PM::vec3 PerspectiveCamera::lookAtPosition() const
	{
		return mLookAt;
	}

	Ray PerspectiveCamera::constructRay(float nx, float ny) const
	{
		float cx = - scale() * nx;
		float cy = - scale() * ny;

		PM::vec3 off = PM::pm_Add(PM::pm_Scale(mRight_Cache, cx), PM::pm_Scale(mUp_Cache, cy));
		PM::vec3 camPos = PM::pm_Add(off, PM::pm_Scale(mDirection_Cache, mLensDistance));
		PM::vec3 rayDir = PM::pm_SetW(PM::pm_Normalize3D(camPos), 0);

		return Ray(position(), rayDir);
	}

	// Cache
	void PerspectiveCamera::onPreRender()
	{
		mDirection_Cache = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Subtract(mLookAt, position())), 0);
		if (PM::pm_MagnitudeSqr3D(mDirection_Cache) < PM_EPSILON)
			mDirection_Cache = PM::pm_Set(0, 0, 1);

		float dot = PM::pm_Dot3D(mDirection_Cache, PM::pm_Set(0, 1, 0));

		if (dot >= 1)
			mRight_Cache = PM::pm_Set(mWidth, 0, 0);
		else if (dot <= -1)
			mRight_Cache = PM::pm_Set(-mWidth, 0, 0);
		else
			mRight_Cache = PM::pm_Scale(
				PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Cross3D(mDirection_Cache, PM::pm_Set(0, 1, 0))), 0),
				mWidth);

		mUp_Cache = PM::pm_Scale(PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Cross3D(mRight_Cache, mDirection_Cache)), 0), mHeight);
	}
}