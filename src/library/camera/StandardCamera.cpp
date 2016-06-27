#include "StandardCamera.h"
#include "ray/Ray.h"

namespace PR
{
	StandardCamera::StandardCamera(const std::string& name, Entity* parent) :
		Camera(name, parent), mOrthographic(false), mWidth(1), mHeight(1),
		mLookAt(PM::pm_Set(0,0,1,1)), mFStop(0), mApertureRadius(0.1f)
	{
	}

	StandardCamera::~StandardCamera()
	{
	}

	std::string StandardCamera::type() const
	{
		return "standardCamera";
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

	void StandardCamera::lookAt(const PM::vec3& v)
	{
		mLookAt = v;
	}

	PM::vec3 StandardCamera::lookAtPosition() const
	{
		return mLookAt;
	}

	void StandardCamera::setFStop(float f)
	{
		mFStop = f;
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
		float cx = - scale() * nx;
		float cy = - scale() * ny;

		if (mOrthographic)
		{
			return Ray(PM::pm_Add(position(), PM::pm_Add(PM::pm_Scale(mRight_Cache, cx), PM::pm_Scale(mUp_Cache, cy))),
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

			PM::vec3 dofOff = PM::pm_Add(PM::pm_Scale(mXApertureRadius_Cache, s), PM::pm_Scale(mYApertureRadius_Cache, c));

			PM::vec3 viewPlane = PM::pm_Add(PM::pm_Scale(mRight_Cache, cx), PM::pm_Scale(mUp_Cache, cy));
			PM::vec3 eyePoint = PM::pm_Add(position(), dofOff);
			PM::vec3 rayDir = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Subtract(viewPlane, eyePoint)), 0);

			return Ray(eyePoint, rayDir);
		}
	}

	// Cache
	void StandardCamera::onPreRender()
	{
		PM::vec3 L = PM::pm_Subtract(mLookAt, position());
		mDirection_Cache = PM::pm_SetW(PM::pm_Normalize3D(L), 0);
		if (PM::pm_MagnitudeSqr3D(mDirection_Cache) <= PM_EPSILON)
			mDirection_Cache = PM::pm_Set(0, 0, 1);

		float dot = PM::pm_Dot3D(mDirection_Cache, PM::pm_Set(0, 1, 0));

		if (dot >= 1)
			mRight_Cache = PM::pm_Set(1, 0, 0);
		else if (dot <= -1)
			mRight_Cache = PM::pm_Set(-1, 0, 0);
		else
			mRight_Cache = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Cross3D(mDirection_Cache, PM::pm_Set(0, 1, 0))), 0);

		mUp_Cache = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Cross3D(mRight_Cache, mDirection_Cache)), 0);

		if (std::abs(mFStop) <= PM_EPSILON || mApertureRadius <= PM_EPSILON)// No depth of field
		{
			mFocalDistance_Cache = 0.0f;
			mXApertureRadius_Cache = PM::pm_Zero();
			mYApertureRadius_Cache = PM::pm_Zero();
			mRight_Cache = PM::pm_Scale(mRight_Cache, mWidth);
			mUp_Cache = PM::pm_Scale(mUp_Cache, mHeight);
		}
		else
		{
			mFocalDistance_Cache = mFStop * PM::pm_Magnitude3D(L);
			mXApertureRadius_Cache = PM::pm_Scale(mRight_Cache, mApertureRadius);
			mYApertureRadius_Cache = PM::pm_Scale(mUp_Cache, mApertureRadius);
			mRight_Cache = PM::pm_Scale(mRight_Cache, mWidth);
			mUp_Cache = PM::pm_Scale(mUp_Cache, mHeight);
		}
	}
}