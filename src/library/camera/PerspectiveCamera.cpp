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
		float cx = -nx * mWidth;
		float cy = -ny * mHeight;

		// Could be cached!
		PM::vec3 dir = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Subtract(mLookAt, position())), 0);

		float dot = PM::pm_Dot3D(dir, PM::pm_Set(0, 1, 0));
		PM::vec3 right;
		if (dot >= 1)
			right = PM::pm_Set(1, 0, 0);
		else if (dot <= -1)
			right = PM::pm_Set(-1, 0, 0);
		else
			right = PM::pm_Cross3D(dir, PM::pm_Set(0, 1, 0));

		PM::vec3 up = PM::pm_Cross3D(right, dir);

		PM::vec3 off = PM::pm_Add(PM::pm_Scale(right, cx), PM::pm_Scale(up, cy));
		PM::vec3 camPos = PM::pm_Add(off, PM::pm_Scale(dir, mLensDistance));
		PM::vec3 rayDir = PM::pm_SetW(PM::pm_Normalize3D(camPos), 0);

		return Ray(position(), rayDir);
	}
}