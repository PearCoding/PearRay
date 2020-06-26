#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "camera/ICamera.h"
#include "camera/ICameraPlugin.h"

#include "ray/Ray.h"
#include "renderer/RenderContext.h"

namespace PR {
constexpr float NEAR_DEFAULT = 0.000001f;
constexpr float FAR_DEFAULT	 = std::numeric_limits<float>::infinity();

class OrthoCamera : public ICamera {
public:
	ENTITY_CLASS

	OrthoCamera(uint32 id, const std::string& name,
				float w, float h, float nearT, float farT,
				const Vector3f& ld, const Vector3f& lr, const Vector3f& lu)
		: ICamera(id, name)
		, mWidth(w)
		, mHeight(h)
		, mNearT(nearT)
		, mFarT(farT)
		, mLocalDirection(ld)
		, mLocalRight(lr)
		, mLocalUp(lu)
	{
	}

	virtual ~OrthoCamera()
	{
	}

	std::string type() const override
	{
		return "orthographic";
	}

	Ray constructRay(const CameraSample& sample) const override
	{
		const float nx = 2 * (sample.Pixel[0] / sample.SensorSize.Width - 0.5f);
		const float ny = 2 * (sample.Pixel[1] / sample.SensorSize.Height - 0.5f);

		Ray ray;
		constructRay(nx, ny, ray.Origin, ray.Direction);

		ray.WavelengthNM = sample.WavelengthNM;
		ray.Weight		 = SpectralBlob(sample.Weight, sample.Weight, sample.Weight, sample.Weight);
		ray.Time		 = sample.Time;
		ray.Flags		 = RF_Camera;
		ray.MinT		 = mNearT;
		ray.MaxT		 = mFarT;

		//ray.normalize();

		return ray;
	}

	void constructRay(float nx, float ny,
					  Vector3f& o, Vector3f& d) const
	{
		o = transform().translation() + mRight_Cache * nx + mUp_Cache * ny;
		d = mDirection_Cache;
	}

	// Cache
	void beforeSceneBuild() override
	{
		ICamera::beforeSceneBuild();

		mDirection_Cache = (normalMatrix() * mLocalDirection).normalized();
		mRight_Cache	 = (normalMatrix() * mLocalRight).normalized() * 0.5f * mWidth;
		mUp_Cache		 = (normalMatrix() * mLocalUp).normalized() * 0.5f * mHeight;

		PR_LOG(L_DEBUG) << name() << ": Dir" << PR_FMT_MAT(mDirection_Cache)
						<< " Right" << PR_FMT_MAT(mRight_Cache)
						<< " Up" << PR_FMT_MAT(mUp_Cache) << std::endl;
	}

private:
	const float mWidth;
	const float mHeight;

	const float mNearT;
	const float mFarT;

	const Vector3f mLocalDirection;
	const Vector3f mLocalRight;
	const Vector3f mLocalUp;

	// Cache:
	Vector3f mDirection_Cache;
	Vector3f mRight_Cache;
	Vector3f mUp_Cache;
};

class OrthoCameraPlugin : public ICameraPlugin {
public:
	std::shared_ptr<ICamera> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;
		std::string name			 = params.getString("name", "__unnamed__");

		return std::make_shared<OrthoCamera>(id, name,
											 params.getNumber("width", 1),
											 params.getNumber("height", 1),
											 params.getNumber("near", NEAR_DEFAULT),
											 params.getNumber("far", FAR_DEFAULT),
											 params.getVector3f("localDirection", Vector3f(0, 0, 1)),
											 params.getVector3f("localRight", Vector3f(1, 0, 0)),
											 params.getVector3f("localUp", Vector3f(0, 1, 0)));
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "ortho", "orthographic" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::OrthoCameraPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)