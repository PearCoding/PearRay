#include "StandardCamera.h"
#include "ray/Ray.h"

#include "performance/Performance.h"

namespace PR
{
	StandardCamera::StandardCamera(uint32 id, const std::string& name) :
		Camera(id, name), mOrthographic(false), mWidth(1), mHeight(1),
		mFStop(0), mApertureRadius(0.05f),
		mLocalDirection(0,0,1), mLocalRight(1,0,0), mLocalUp(0,1,0)
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
		mWidth = 2 * std::tan(foh / 2);
		mHeight = 2 * std::tan(fov / 2);
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

	void StandardCamera::setLocalDirection(const Eigen::Vector3f& d)
	{
		mLocalDirection = d;
	}

	Eigen::Vector3f StandardCamera::localDirection() const
	{
		return mLocalDirection;
	}

	void StandardCamera::setLocalRight(const Eigen::Vector3f& d)
	{
		mLocalRight = d;
	}

	Eigen::Vector3f StandardCamera::localRight() const
	{
		return mLocalRight;
	}

	void StandardCamera::setLocalUp(const Eigen::Vector3f& d)
	{
		mLocalUp = d;
	}

	Eigen::Vector3f StandardCamera::localUp() const
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
			return Ray(Eigen::Vector2i(0,0),
					position() + mRight_Cache * nx + mUp_Cache * ny,
					mDirection_Cache,
					0,t,wavelength);
		}
		else
		{
			Eigen::Vector3f viewPlane = mRight_Cache * nx + mUp_Cache * ny + mFocalDistance_Cache;

			if (mHasDOF_Cache)
			{
				float s = std::sin(2*PR_PI*rx);
				float c = std::cos(2*PR_PI*rx);
				Eigen::Vector3f eyePoint = mXApertureRadius_Cache*ry*s+
					mYApertureRadius_Cache*ry*c;
				Eigen::Vector3f rayDir = (viewPlane-eyePoint).normalized();

				return Ray(Eigen::Vector2i(0,0), // Will be set by render context
					position()+eyePoint,
					rayDir,
					0,t,wavelength);
			}
			else
			{
				return Ray(Eigen::Vector2i(0,0), // Will be set by render context
					position(),
					viewPlane.normalized(),
					0,t,wavelength);
			}
		}
	}

	// Cache
	void StandardCamera::onFreeze()
	{
		PR_GUARD_PROFILE();

		Camera::onFreeze();

		mDirection_Cache = (directionMatrix()*mLocalDirection).normalized();
		mRight_Cache = (directionMatrix()*mLocalRight).normalized();
		mUp_Cache = (directionMatrix()*mLocalUp).normalized();

		PR_LOGGER.logf(L_Info, M_Camera,"%s: Dir[%.3f,%.3f,%.3f] Right[%.3f,%.3f,%.3f] Up[%.3f,%.3f,%.3f]",
			name().c_str(),
			mDirection_Cache(0), mDirection_Cache(1), mDirection_Cache(2),
			mRight_Cache(0), mRight_Cache(1), mRight_Cache(2),
			mUp_Cache(0), mUp_Cache(1), mUp_Cache(2));

		if (mOrthographic || std::abs(mFStop) <= PR_EPSILON || mApertureRadius <= PR_EPSILON)// No depth of field
		{
			mFocalDistance_Cache = mDirection_Cache;
			mXApertureRadius_Cache = Eigen::Vector3f(0,0,0);
			mYApertureRadius_Cache = Eigen::Vector3f(0,0,0);
			mRight_Cache *= 0.5f*mWidth;
			mUp_Cache *= 0.5f*mHeight;
			mHasDOF_Cache = false;
		}
		else
		{
			mFocalDistance_Cache = mDirection_Cache*(mFStop+1);
			mXApertureRadius_Cache = mRight_Cache*mApertureRadius;
			mYApertureRadius_Cache = mUp_Cache*mApertureRadius;
			mRight_Cache *= 0.5f*mWidth*(mFStop+1);
			mUp_Cache *= 0.5f*mHeight*(mFStop+1);
			mHasDOF_Cache = true;

			PR_LOGGER.logf(L_Info, M_Camera,"    FocalDistance[%.3f,%.3f,%.3f] XAperature[%.3f,%.3f,%.3f] YAperature[%.3f,%.3f,%.3f]",
				name().c_str(),
				mFocalDistance_Cache(0), mFocalDistance_Cache(1), mFocalDistance_Cache(2),
				mXApertureRadius_Cache(0), mXApertureRadius_Cache(1), mXApertureRadius_Cache(2),
				mYApertureRadius_Cache(0), mYApertureRadius_Cache(1), mYApertureRadius_Cache(2));
		}
	}
}
