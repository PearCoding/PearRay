#include "camera/ICamera.h"

#include "Logger.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"

namespace PR {
class PR_LIB StandardCamera : public ICamera {
public:
	ENTITY_CLASS

	StandardCamera(uint32 id, const std::string& name);
	virtual ~StandardCamera();

	void setWithAngle(float foh, float fov);
	void setWithSize(float width, float height);

	void setWidth(float w);
	float width() const;

	void setHeight(float h);
	float height() const;

	void setLocalDirection(const Eigen::Vector3f& d);
	Eigen::Vector3f localDirection() const;

	void setLocalRight(const Eigen::Vector3f& d);
	Eigen::Vector3f localRight() const;

	void setLocalUp(const Eigen::Vector3f& d);
	Eigen::Vector3f localUp() const;

	// Depth of Field
	void setFStop(float f);
	float fstop() const;

	void setApertureRadius(float f);
	float apertureRadius() const;

	// ICamera
	RayPackage constructRay(const CameraSample& sample) const override;

	// VirtualEntity
	std::string type() const override;

protected:
	void onFreeze(RenderContext* context) override; // Cache

private:
	void constructRay(const vfloat& nx, const vfloat& ny,
					  const vfloat& r1, const vfloat& r2,
					  vfloat& ox, vfloat& oy, vfloat& oz,
					  vfloat& dx, vfloat& dy, vfloat& dz) const;

	float mWidth;
	float mHeight;

	float mFStop;
	float mApertureRadius;

	Eigen::Vector3f mLocalDirection;
	Eigen::Vector3f mLocalRight;
	Eigen::Vector3f mLocalUp;

	// Cache:
	Eigen::Vector3f mDirection_Cache;
	Eigen::Vector3f mRight_Cache;
	Eigen::Vector3f mUp_Cache;

	bool mHasDOF_Cache;
	Eigen::Vector3f mFocalDistance_Cache;
	Eigen::Vector3f mXApertureRadius_Cache;
	Eigen::Vector3f mYApertureRadius_Cache;
};

StandardCamera::StandardCamera(uint32 id, const std::string& name)
	: Camera(id, name)
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

RayPackage StandardCamera::cpm(const CameraSample& sample) const
{
	PR_ASSERT(isFrozen(), "has to be frozen");

	const vfloat nx = 2 * (sample.Pixel[0] / sample.SensorSize.x() - 0.5f);
	//const float nx1 = 2 * ((sample.PixelF.x() + 1) / sample.SensorSize.x() - 0.5f);
	const vfloat ny = 2 * (sample.Pixel[1] / sample.SensorSize.y() - 0.5f);
	//const float ny1 = 2 * ((sample.PixelF.y() + 1) / sample.SensorSize.y() - 0.5f);

	vfloat o, d;
	constructRay(nx, ny, sample.R[0], sample.R[1], o, d);

	/*Eigen::Vector3f ox;
	Eigen::Vector3f dx;
	constructRay(nx1, ny, sample.R, ox, dx);

	Eigen::Vector3f oy;
	Eigen::Vector3f dy;
	constructRay(nx, ny1, sample.R, oy, dy);*/

	Ray ray(1, sample.PixelIndex, o, d, 0, sample.Time, sample.WavelengthIndex);
	/*ray.setXOrigin(ox);
	ray.setXDirection(dx);
	ray.setYOrigin(oy);
	ray.setYDirection(dy);*/

	return ray;
}

void StandardCamera::constructRay(const vfloat& nx, const vfloat& ny,
								  const vfloat& r1, const vfloat& r2,
								  vfloat& ox, vfloat& oy, vfloat& oz,
								  vfloat& dx, vfloat& dy, vfloat& dz) const
{
	const Eigen::Vector3f pos = transform().translation();

	const vfloat vx = mRight_Cache(0) * nx + mUp_Cache(0) * ny + mFocalDistance_Cache(0);
	const vfloat vy = mRight_Cache(1) * nx + mUp_Cache(1) * ny + mFocalDistance_Cache(1);
	const vfloat vz = mRight_Cache(2) * nx + mUp_Cache(2) * ny + mFocalDistance_Cache(2);

	if (mHasDOF_Cache) {
		const vfloat t = 2 * PR_PI * r1;
		const vfloat s = std::sin(t);
		const vfloat c = std::cos(t);

		vfloat ex = mXApertureRadius_Cache(0) * r2 * s + mYApertureRadius_Cache(0) * r2 * c;
		vfloat ey = mXApertureRadius_Cache(1) * r2 * s + mYApertureRadius_Cache(1) * r2 * c;
		vfloat ez = mXApertureRadius_Cache(2) * r2 * s + mYApertureRadius_Cache(2) * r2 * c;

		ox = pos(0) + ex;
		oy = pos(1) + ey;
		oz = pos(2) + ez;

		dx = vx - ex;
		dy = vy - ey;
		dz = vz - ez;
		
		normalizeV(dx, dy, dz);
	} else {
		ox = simdpp::make_float(pos(0));
		oy = simdpp::make_float(pos(1));
		oz = simdpp::make_float(pos(2));

		dx = vx;
		dy = vy;
		dz = vz;

		normalizeV(dx, dy, dz);
	}
}

// Cache
void StandardCamera::onFreeze(RenderContext* context)
{
	Camera::onFreeze(context);

	mDirection_Cache = (directionMatrix() * mLocalDirection).normalized();
	mRight_Cache	 = (directionMatrix() * mLocalRight).normalized();
	mUp_Cache		 = (directionMatrix() * mLocalUp).normalized();

	PR_LOG(L_INFO) << name() << ": Dir[" << mDirection_Cache << "] Right[" << mRight_Cache << "] Up[" << mUp_Cache << "]" << std::endl;

	if (mOrthographic || std::abs(mFStop) <= PR_EPSILON || mApertureRadius <= PR_EPSILON) { // No depth of field
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

		PR_LOG(L_INFO) << "    FocalDistance[" << mFocalDistance_Cache
					   << "] XAperature[" << mXApertureRadius_Cache
					   << "] YAperature[" << mYApertureRadius_Cache << "]" << std::endl;
	}
}
} // namespace PR
