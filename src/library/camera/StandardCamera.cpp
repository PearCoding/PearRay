#include "StandardCamera.h"
#include "ray/Ray.h"

#include "performance/Performance.h"

namespace PR
{
	StandardCamera::StandardCamera(const std::string& name, Entity* parent) :
		Camera(name, parent), mOrthographic(false), mWidth(1), mHeight(1),
		mLocalDirection(PM::pm_Set(0,0,1)), mLocalRight(PM::pm_Set(1,0,0)), mLocalUp(PM::pm_Set(0,1,0)),
		mFStop(0), mApertureRadius(0.1f)
	{
	}

	StandardCamera::~StandardCamera()
	{
	}

	std::string StandardCamera::type() const
	{
		return "standard_camera";
	}

	void StandardCamera::setOrthographic(bool b)
	{
		mOrthographic = b;
	}

	bool StandardCamera::isOrthographic() const
	{
		return mOrthographic;
	}

	void StandardCamera::setWithAngle(float foh, float fov)
	{
		mWidth = 2 * tan(foh / 2);
		mHeight = 2 * tan(fov / 2);
	}

	void StandardCamera::setWithSize(float width, float height)
	{
		mWidth = width;
		mHeight = height;
	}

	void StandardCamera::setWidth(float w)
	{
		mWidth = w;
	}

	float StandardCamera::width() const
	{
		return mWidth;
	}

	void StandardCamera::setHeight(float h)
	{
		mHeight = h;
	}

	float StandardCamera::height() const
	{
		return mHeight;
	}

	void StandardCamera::setFStop(float f)
	{
		mFStop = f;
	}

	void StandardCamera::setLocalDirection(const PM::vec3& d)
	{
		mLocalDirection = d;
	}

	PM::vec3 StandardCamera::localDirection() const
	{
		return mLocalDirection;
	}

	void StandardCamera::setLocalRight(const PM::vec3& d)
	{
		mLocalRight = d;
	}

	PM::vec3 StandardCamera::localRight() const
	{
		return mLocalRight;
	}

	void StandardCamera::setLocalUp(const PM::vec3& d)
	{
		mLocalUp = d;
	}

	PM::vec3 StandardCamera::localUp() const
	{
		return mLocalUp;
	}

	float StandardCamera::fstop() const
	{
		return mFStop;
	}

	void StandardCamera::setApertureRadius(float f)
	{
		mApertureRadius = f;
	}

	float StandardCamera::apertureRadius() const
	{
		return mApertureRadius;
	}

	Ray StandardCamera::constructRay(float nx, float ny, float rx, float ry, float t) const
	{
		PR_ASSERT(isFrozen());
		
		PR_GUARD_PROFILE();

		if (mOrthographic)
		{
			return Ray(PM::pm_Add(worldPosition(),
					PM::pm_Add(PM::pm_Scale(mRight_Cache, nx), PM::pm_Scale(mUp_Cache, ny))),
				mDirection_Cache);
		}
		else
		{
			float s, c;
			if (std::abs(mFStop) > PM_EPSILON && mApertureRadius > PM_EPSILON)
			{
				PM::pm_SinCosT(PM_2_PI_F * rx, s, c);
				s *= ry;
				c *= ry;
			}
			else
			{
				s = 0;
				c = 0;
			}

			PM::vec3 eyePoint = PM::pm_Add(
				PM::pm_Scale(mXApertureRadius_Cache, s), PM::pm_Scale(mYApertureRadius_Cache, c));

			PM::vec3 viewPlane = PM::pm_Add(
					PM::pm_Add(PM::pm_Scale(mRight_Cache, nx), PM::pm_Scale(mUp_Cache, ny)),
					mDirection_Cache);// One unit away in z direction.
			PM::vec3 rayDir = PM::pm_Normalize3D(PM::pm_Subtract(viewPlane, eyePoint));

			return Ray(PM::pm_Add(worldPosition(), eyePoint),
					rayDir);
		}
	}

	// Cache
	void StandardCamera::onFreeze()
	{
		PR_GUARD_PROFILE();
		
		Camera::onFreeze();

		mDirection_Cache = PM::pm_SetW(PM::pm_Normalize3D(
			PM::pm_Transform(worldDirectionMatrix(), mLocalDirection)), 0);
		mRight_Cache = PM::pm_SetW(PM::pm_Normalize3D(
			PM::pm_Transform(worldDirectionMatrix(), mLocalRight)), 0);
		mUp_Cache = PM::pm_SetW(PM::pm_Normalize3D(
			PM::pm_Transform(worldDirectionMatrix(), mLocalUp)), 0);

		PR_LOGGER.logf(L_Info, M_Camera,"%s: Dir[%.3f,%.3f,%.3f] Right[%.3f,%.3f,%.3f] Up[%.3f,%.3f,%.3f]",
			name().c_str(),
			PM::pm_GetX(mDirection_Cache), PM::pm_GetY(mDirection_Cache), PM::pm_GetZ(mDirection_Cache),
			PM::pm_GetX(mRight_Cache), PM::pm_GetY(mRight_Cache), PM::pm_GetZ(mRight_Cache),
			PM::pm_GetX(mUp_Cache), PM::pm_GetY(mUp_Cache), PM::pm_GetZ(mUp_Cache));

		if (std::abs(mFStop) <= PM_EPSILON || mApertureRadius <= PM_EPSILON)// No depth of field
		{
			mFocalDistance_Cache = 0.0f;
			mXApertureRadius_Cache = PM::pm_Zero();
			mYApertureRadius_Cache = PM::pm_Zero();
			mRight_Cache = PM::pm_Scale(mRight_Cache, 0.5f*mWidth);
			mUp_Cache = PM::pm_Scale(mUp_Cache, 0.5f*mHeight);
		}
		else
		{
			mFocalDistance_Cache = mFStop;
			mXApertureRadius_Cache = PM::pm_Scale(mRight_Cache, mApertureRadius);
			mYApertureRadius_Cache = PM::pm_Scale(mUp_Cache, mApertureRadius);
			mRight_Cache = PM::pm_Scale(mRight_Cache, 0.5f*mWidth);
			mUp_Cache = PM::pm_Scale(mUp_Cache, 0.5f*mHeight);
		}
	}
}