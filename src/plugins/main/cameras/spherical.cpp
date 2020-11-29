#include "math/Spherical.h"
#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "camera/ICamera.h"
#include "camera/ICameraPlugin.h"
#include "math/Tangent.h"

#include "ray/Ray.h"
#include "renderer/RenderContext.h"

namespace PR {
constexpr float NEAR_DEFAULT = 0.000001f;
constexpr float FAR_DEFAULT	 = PR_INF;

class SphericalCamera : public ICamera {
public:
	ENTITY_CLASS

	SphericalCamera(const std::string& name, const Transformf& transform,
					float thetaStart, float thetaEnd, float phiStart, float phiEnd, float nearT, float farT,
					const Vector3f& ld, const Vector3f& lr, const Vector3f& lu)
		: ICamera(name, transform)
		, mThetaStart(thetaStart)
		, mThetaEnd(thetaEnd)
		, mPhiStart(phiStart)
		, mPhiEnd(phiEnd)
		, mNearT(nearT)
		, mFarT(farT)
		, mLocalDirection(ld)
		, mLocalRight(lr)
		, mLocalUp(lu)
		, mDirection_Cache((normalMatrix() * mLocalDirection).normalized())
		, mRight_Cache((normalMatrix() * mLocalRight).normalized())
		, mUp_Cache((normalMatrix() * mLocalUp).normalized())
	{
		PR_LOG(L_DEBUG) << name << ": Dir" << PR_FMT_MAT(mDirection_Cache)
						<< " Right" << PR_FMT_MAT(mRight_Cache)
						<< " Up" << PR_FMT_MAT(mUp_Cache) << std::endl;
	}

	virtual ~SphericalCamera()
	{
	}

	std::string type() const override
	{
		return "spherical";
	}

	std::optional<CameraRay> constructRay(const CameraSample& sample) const override
	{
		const float nx = sample.Pixel[0] / sample.SensorSize.Width;	 // [0,1]
		const float ny = sample.Pixel[1] / sample.SensorSize.Height; // [0,1]

		CameraRay ray;
		constructRay(nx, 1 - ny, ray.Origin, ray.Direction);

		ray.MinT = mNearT;
		ray.MaxT = mFarT;

		return ray;
	}

	inline void constructRay(float nx, float ny,
							 Vector3f& o, Vector3f& d) const
	{
		o				  = transform().translation();
		const float theta = mThetaStart + ny * (mThetaEnd - mThetaStart);
		const float phi	  = mPhiStart + nx * (mPhiEnd - mPhiStart);

		const float sT = std::sin(theta);
		const float cT = std::cos(theta);
		const float sP = std::sin(phi);
		const float cP = std::cos(phi);
		d			   = Tangent::fromTangentSpace(mUp_Cache, mRight_Cache, mDirection_Cache,
									   Vector3f(sP * cT, cP * cT, sT));
	}

private:
	const float mThetaStart;
	const float mThetaEnd;
	const float mPhiStart;
	const float mPhiEnd;

	const float mNearT;
	const float mFarT;

	const Vector3f mLocalDirection;
	const Vector3f mLocalRight;
	const Vector3f mLocalUp;

	// Cache:
	const Vector3f mDirection_Cache;
	const Vector3f mRight_Cache;
	const Vector3f mUp_Cache;
};

class SphericalCameraPlugin : public ICameraPlugin {
public:
	std::shared_ptr<ICamera> create(const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.parameters();
		std::string name			 = params.getString("name", "__unnamed__");

		return std::make_shared<SphericalCamera>(name, ctx.transform(),
												 params.getNumber("theta_start", 0.0f),
												 params.getNumber("theta_end", PR_PI_2),
												 params.getNumber("phi_start", -PR_PI),
												 params.getNumber("phi_end", PR_PI),
												 params.getNumber("near", NEAR_DEFAULT),
												 params.getNumber("far", FAR_DEFAULT),
												 params.getVector3f("localDirection", ICamera::DefaultDirection),
												 params.getVector3f("localRight", ICamera::DefaultRight),
												 params.getVector3f("localUp", ICamera::DefaultUp));
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "spherical" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SphericalCameraPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)