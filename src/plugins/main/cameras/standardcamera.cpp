#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "camera/ICamera.h"
#include "camera/ICameraFactory.h"
#include "ray/RayPackage.h"
#include "renderer/RenderContext.h"

namespace PR {
class StandardCamera : public ICamera {
public:
	ENTITY_CLASS

	StandardCamera(uint32 id, const std::string& name)
		: ICamera(id, name)
		, mWidth(1)
		, mHeight(1)
		, mFStop(0)
		, mApertureRadius(0.05f)
		, mLocalDirection(0, 0, 1)
		, mLocalRight(1, 0, 0)
		, mLocalUp(0, 1, 0)
		, mHasDOF_Cache(false)
	{
	}

	virtual ~StandardCamera()
	{
	}

	std::string type() const override
	{
		return "standard_camera";
	}

	void setWithAngle(float foh, float fov)
	{
		mWidth  = 2 * std::tan(foh / 2);
		mHeight = 2 * std::tan(fov / 2);
	}

	void setWithSize(float width, float height)
	{
		mWidth  = width;
		mHeight = height;
	}

	void setWidth(float w)
	{
		mWidth = w;
	}

	void setHeight(float h)
	{
		mHeight = h;
	}

	void setFStop(float f)
	{
		mFStop = f;
	}

	void setLocalDirection(const Vector3f& d)
	{
		mLocalDirection = d;
	}

	void setLocalRight(const Vector3f& d)
	{
		mLocalRight = d;
	}

	void setLocalUp(const Vector3f& d)
	{
		mLocalUp = d;
	}

	void setApertureRadius(float f)
	{
		mApertureRadius = f;
	}

	Ray constructRay(const CameraSample& sample) const override
	{
		const float nx = 2 * (sample.Pixel[0] / sample.SensorSize.x() - 0.5f);
		const float ny = 2 * (sample.Pixel[1] / sample.SensorSize.y() - 0.5f);

		Ray ray;
		constructRay(nx, ny, sample.R[0], sample.R[1],
					 ray.Origin, ray.Direction);

		ray.WavelengthIndex = sample.WavelengthIndex;
		ray.Weight			= ColorTriplet(sample.Weight, sample.Weight, sample.Weight);
		ray.Time			= sample.Time;
		ray.Flags			= RF_Camera;

		//ray.normalize();

		return ray;
	}

	void constructRay(float nx, float ny,
					  float r1, float r2,
					  Vector3f& o, Vector3f& d) const
	{
		o = transform().translation();
		d = mRight_Cache * nx + mUp_Cache * ny + mFocalDistance_Cache;

		if (mHasDOF_Cache) {
			const float t = 2 * PR_PI * r1;
			float s		  = std::sin(t);
			float c		  = std::cos(t);

			const Vector3f e = mXApertureRadius_Cache * r2 * s + mYApertureRadius_Cache * r2 * c;

			o += e;
			d -= e;
		}

		d.normalize();
	}

	// Cache
	void beforeSceneBuild() override
	{
		ICamera::beforeSceneBuild();

		mDirection_Cache = (normalMatrix() * mLocalDirection).normalized();
		mRight_Cache	 = (normalMatrix() * mLocalRight).normalized();
		mUp_Cache		 = (normalMatrix() * mLocalUp).normalized();

		PR_LOG(L_INFO) << name() << ": Dir" << PR_FMT_MAT(mDirection_Cache)
					   << " Right" << PR_FMT_MAT(mRight_Cache)
					   << " Up" << PR_FMT_MAT(mUp_Cache) << std::endl;

		// No depth of field
		if (std::abs(mFStop) <= PR_EPSILON || mApertureRadius <= PR_EPSILON) {
			mFocalDistance_Cache   = mDirection_Cache;
			mXApertureRadius_Cache = Vector3f(0, 0, 0);
			mYApertureRadius_Cache = Vector3f(0, 0, 0);
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

			PR_LOG(L_INFO) << "    FocalDistance" << PR_FMT_MAT(mFocalDistance_Cache)
						   << " XAperature" << PR_FMT_MAT(mXApertureRadius_Cache)
						   << " YAperature" << PR_FMT_MAT(mYApertureRadius_Cache) << std::endl;
		}
	}

private:
	float mWidth;
	float mHeight;

	float mFStop;
	float mApertureRadius;

	Vector3f mLocalDirection;
	Vector3f mLocalRight;
	Vector3f mLocalUp;

	// Cache:
	Vector3f mDirection_Cache;
	Vector3f mRight_Cache;
	Vector3f mUp_Cache;

	bool mHasDOF_Cache;
	Vector3f mFocalDistance_Cache;
	Vector3f mXApertureRadius_Cache;
	Vector3f mYApertureRadius_Cache;
};

class StandardCameraFactory : public ICameraFactory {
public:
	std::shared_ptr<ICamera> create(uint32 id, uint32 uuid, const SceneLoadContext& ctx)
	{
		Registry& reg	= ctx.Env->registry();
		std::string name = reg.getForObject<std::string>(RG_CAMERA, uuid, "name", "__unnamed__");

		auto cam = std::make_shared<StandardCamera>(id, name);

		cam->setLocalDirection(reg.getForObject<Vector3f>(RG_CAMERA, uuid, "localDirection", Vector3f(0, 0, 1)));
		cam->setLocalUp(reg.getForObject<Vector3f>(RG_CAMERA, uuid, "localUp", Vector3f(0, 1, 0)));
		cam->setLocalRight(reg.getForObject<Vector3f>(RG_CAMERA, uuid, "localRight", Vector3f(1, 0, 0)));

		cam->setWidth(reg.getForObject<float>(RG_CAMERA, uuid, "width", 1));
		cam->setHeight(reg.getForObject<float>(RG_CAMERA, uuid, "height", 1));

		cam->setApertureRadius(reg.getForObject<float>(RG_CAMERA, uuid, "apertureRadius", 1));
		cam->setFStop(reg.getForObject<float>(RG_CAMERA, uuid, "fstop", 0));

		return cam;
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "standard_camera", "standard", "default" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::StandardCameraFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)