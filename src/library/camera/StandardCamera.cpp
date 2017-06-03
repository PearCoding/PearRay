#include "StandardCamera.h"
#include "performance/Performance.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"

namespace PR {
StandardCamera::StandardCamera(uint32 id, const std::string& name)
	: Camera(id, name)
	, mOrthographic(false)
	, mWidth(1)
	, mHeight(1)
	, mFStop(0)
	, mApertureRadius(0.05f)
	, mLocalDirection(0, 0, 1)
	, mLocalRight(1, 0, 0)
	, mLocalUp(0, 1, 0)
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
	mWidth  = 2 * std::tan(foh / 2);
	mHeight = 2 * std::tan(fov / 2);
}

void StandardCamera::setWithSize(float width, float height)
{
	mWidth  = width;
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

Ray StandardCamera::constructRay(RenderContext* context, const CameraSample& sample) const
{
	PR_ASSERT(isFrozen(), "has to be frozen");

	PR_GUARD_PROFILE();

	const float nx  = 2 * (sample.PixelF.x() / context->fullWidth() - 0.5f);
	const float nx1 = 2 * ((sample.PixelF.x() + 1) / context->fullWidth() - 0.5f);
	const float ny  = 2 * (sample.PixelF.y() / context->fullHeight() - 0.5f);
	const float ny1 = 2 * ((sample.PixelF.y() + 1) / context->fullHeight() - 0.5f);

	Eigen::Vector3f o;
	Eigen::Vector3f d;
	constructRay(nx, ny, sample.R, o, d);

	Eigen::Vector3f ox;
	Eigen::Vector3f dx;
	constructRay(nx1, ny, sample.R, ox, dx);

	Eigen::Vector3f oy;
	Eigen::Vector3f dy;
	constructRay(nx, ny1, sample.R, oy, dy);

	Ray ray(sample.Pixel, o, d, 0, sample.Time, sample.WavelengthIndex);
	ray.setXOrigin(ox);
	ray.setXDirection(dx);
	ray.setYOrigin(oy);
	ray.setYDirection(dy);

	return ray;
}

void StandardCamera::constructRay(float nx, float ny, const Eigen::Vector2f& r, Eigen::Vector3f& origin, Eigen::Vector3f& dir) const
{
	if (mOrthographic) {
		origin = position() + mRight_Cache * nx + mUp_Cache * ny;
		dir	= mDirection_Cache;
	} else {
		Eigen::Vector3f viewPlane = mRight_Cache * nx + mUp_Cache * ny + mFocalDistance_Cache;

		if (mHasDOF_Cache) {
			float s					 = std::sin(2 * PR_PI * r.x());
			float c					 = std::cos(2 * PR_PI * r.x());
			Eigen::Vector3f eyePoint = mXApertureRadius_Cache * r.y() * s + mYApertureRadius_Cache * r.y() * c;
			Eigen::Vector3f rayDir   = (viewPlane - eyePoint).normalized();

			origin = position() + eyePoint;
			dir	= rayDir;
		} else {
			origin = position();
			dir	= viewPlane.normalized();
		}
	}
}

// Cache
void StandardCamera::onFreeze()
{
	PR_GUARD_PROFILE();

	Camera::onFreeze();

	mDirection_Cache = (directionMatrix() * mLocalDirection).normalized();
	mRight_Cache	 = (directionMatrix() * mLocalRight).normalized();
	mUp_Cache		 = (directionMatrix() * mLocalUp).normalized();

	PR_LOGGER.logf(L_Info, M_Camera, "%s: Dir[%.3f,%.3f,%.3f] Right[%.3f,%.3f,%.3f] Up[%.3f,%.3f,%.3f]",
				   name().c_str(),
				   mDirection_Cache(0), mDirection_Cache(1), mDirection_Cache(2),
				   mRight_Cache(0), mRight_Cache(1), mRight_Cache(2),
				   mUp_Cache(0), mUp_Cache(1), mUp_Cache(2));

	if (mOrthographic || std::abs(mFStop) <= PR_EPSILON || mApertureRadius <= PR_EPSILON) // No depth of field
	{
		mFocalDistance_Cache   = mDirection_Cache;
		mXApertureRadius_Cache = Eigen::Vector3f(0, 0, 0);
		mYApertureRadius_Cache = Eigen::Vector3f(0, 0, 0);
		mRight_Cache *= 0.5f * mWidth;
		mUp_Cache *= 0.5f * mHeight;
		mHasDOF_Cache = false;
	} else {
		mFocalDistance_Cache   = mDirection_Cache * (mFStop + 1);
		mXApertureRadius_Cache = mRight_Cache * mApertureRadius;
		mYApertureRadius_Cache = mUp_Cache * mApertureRadius;
		mRight_Cache *= 0.5f * mWidth * (mFStop + 1);
		mUp_Cache *= 0.5f * mHeight * (mFStop + 1);
		mHasDOF_Cache = true;

		PR_LOGGER.logf(L_Info, M_Camera, "    FocalDistance[%.3f,%.3f,%.3f] XAperature[%.3f,%.3f,%.3f] YAperature[%.3f,%.3f,%.3f]",
					   name().c_str(),
					   mFocalDistance_Cache(0), mFocalDistance_Cache(1), mFocalDistance_Cache(2),
					   mXApertureRadius_Cache(0), mXApertureRadius_Cache(1), mXApertureRadius_Cache(2),
					   mYApertureRadius_Cache(0), mYApertureRadius_Cache(1), mYApertureRadius_Cache(2));
	}
}
}
