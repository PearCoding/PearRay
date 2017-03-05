#include "StandardCamera.h"
#include "ray/Ray.h"

#include "performance/Performance.h"

namespace PR
{
	StandardCamera::StandardCamera(uint32 id, const std::string& name) :
		Camera(id, name), mOrthographic(false), mWidth(1), mHeight(1),
		mFStop(0), mApertureRadius(0.05f),
		mLocalDirection(PM::pm_Set(0,0,1)), mLocalRight(PM::pm_Set(1,0,0)), mLocalUp(PM::pm_Set(0,1,0))
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

	// nx, ny -> [-1,1]
	// rx, ry -> [0, 1]
	// t -> [0, inf]
	Ray StandardCamera::constructRay(float nx, float ny, float rx, float ry, float t, uint8 wavelength) const
	{
		PR_ASSERT(isFrozen(), "has to be frozen");

		PR_GUARD_PROFILE();

		if (mOrthographic)
		{
			return Ray(0,0,
					PM::pm_Add(position(),
						PM::pm_Add(PM::pm_Scale(mRight_Cache, nx), PM::pm_Scale(mUp_Cache, ny))),
					mDirection_Cache,
					0,t,wavelength);
		}
		else
		{
			PM::vec3 viewPlane = PM::pm_Add(
					PM::pm_Add(PM::pm_Scale(mRight_Cache, nx),
						PM::pm_Scale(mUp_Cache, ny)),
					mFocalDistance_Cache);

			if (mHasDOF_Cache)
			{
				float s, c;
				PM::pm_SinCos(PM_2_PI_F * rx, s, c);
				PM::vec3 eyePoint = PM::pm_Add(PM::pm_Scale(mXApertureRadius_Cache, ry*s),
					PM::pm_Scale(mYApertureRadius_Cache, ry*c));
				PM::vec3 rayDir = PM::pm_QualityNormalize(PM::pm_Subtract(viewPlane, eyePoint));

				return Ray(0,0, // Will be set by render context
					PM::pm_Add(position(), eyePoint),
					rayDir,
					0,t,wavelength);
			}
			else
			{
				return Ray(0,0, // Will be set by render context
					position(),
					PM::pm_QualityNormalize(viewPlane),
					0,t,wavelength);
			}
		}
	}

	// Cache
	void StandardCamera::onFreeze()
	{
		PR_GUARD_PROFILE();

		Camera::onFreeze();

		mDirection_Cache = PM::pm_QualityNormalize(
			PM::pm_Transform(directionMatrix(), mLocalDirection));
		mRight_Cache = PM::pm_QualityNormalize(
			PM::pm_Transform(directionMatrix(), mLocalRight));
		mUp_Cache = PM::pm_QualityNormalize(
			PM::pm_Transform(directionMatrix(), mLocalUp));

		PR_LOGGER.logf(L_Info, M_Camera,"%s: Dir[%.3f,%.3f,%.3f] Right[%.3f,%.3f,%.3f] Up[%.3f,%.3f,%.3f]",
			name().c_str(),
			PM::pm_GetX(mDirection_Cache), PM::pm_GetY(mDirection_Cache), PM::pm_GetZ(mDirection_Cache),
			PM::pm_GetX(mRight_Cache), PM::pm_GetY(mRight_Cache), PM::pm_GetZ(mRight_Cache),
			PM::pm_GetX(mUp_Cache), PM::pm_GetY(mUp_Cache), PM::pm_GetZ(mUp_Cache));

		if (mOrthographic || std::abs(mFStop) <= PM_EPSILON || mApertureRadius <= PM_EPSILON)// No depth of field
		{
			mFocalDistance_Cache = mDirection_Cache;
			mXApertureRadius_Cache = PM::pm_Zero3D();
			mYApertureRadius_Cache = PM::pm_Zero3D();
			mRight_Cache = PM::pm_Scale(mRight_Cache, 0.5f*mWidth);
			mUp_Cache = PM::pm_Scale(mUp_Cache, 0.5f*mHeight);
			mHasDOF_Cache = false;
		}
		else
		{
			mFocalDistance_Cache = PM::pm_Scale(mDirection_Cache, mFStop+1);
			mXApertureRadius_Cache = PM::pm_Scale(mRight_Cache, mApertureRadius);
			mYApertureRadius_Cache = PM::pm_Scale(mUp_Cache, mApertureRadius);
			mRight_Cache = PM::pm_Scale(mRight_Cache, 0.5f*mWidth*(mFStop+1));
			mUp_Cache = PM::pm_Scale(mUp_Cache, 0.5f*mHeight*(mFStop+1));
			mHasDOF_Cache = true;

			PR_LOGGER.logf(L_Info, M_Camera,"    FocalDistance[%.3f,%.3f,%.3f] XAperature[%.3f,%.3f,%.3f] YAperature[%.3f,%.3f,%.3f]",
				name().c_str(),
				PM::pm_GetX(mFocalDistance_Cache), PM::pm_GetY(mFocalDistance_Cache), PM::pm_GetZ(mFocalDistance_Cache),
				PM::pm_GetX(mXApertureRadius_Cache), PM::pm_GetY(mXApertureRadius_Cache), PM::pm_GetZ(mXApertureRadius_Cache),
				PM::pm_GetX(mYApertureRadius_Cache), PM::pm_GetY(mYApertureRadius_Cache), PM::pm_GetZ(mYApertureRadius_Cache));
		}
	}
}
