#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "camera/ICamera.h"
#include "camera/ICameraPlugin.h"

#include "ray/Ray.h"
#include "renderer/RenderContext.h"

namespace PR {
constexpr float NEAR_DEFAULT = 0.000001f;
constexpr float FAR_DEFAULT	 = PR_INF;

template <bool HasDOF>
class PerspectiveCamera : public ICamera {
public:
	ENTITY_CLASS

	PerspectiveCamera(const std::string& name, const Transformf& transform,
					  float w, float h, float fstop, float apert, float nearT, float farT,
					  const Vector3f& ld, const Vector3f& lr, const Vector3f& lu)
		: ICamera(name, transform)
		, mWidth(w)
		, mHeight(h)
		, mFStop(fstop)
		, mApertureRadius(apert)
		, mNearT(nearT)
		, mFarT(farT)
		, mLocalDirection(ld)
		, mLocalRight(lr)
		, mLocalUp(lu)
	{
		cache();
	}

	virtual ~PerspectiveCamera()
	{
	}

	std::string type() const override
	{
		return "perspective";
	}

	std::optional<CameraRay> constructRay(const CameraSample& sample) const override
	{
		const float nx = 2 * (sample.Pixel[0] / sample.SensorSize.Width - 0.5f);
		const float ny = 2 * (sample.Pixel[1] / sample.SensorSize.Height - 0.5f);

		CameraRay ray;
		constructRay(nx, -ny, sample.Lens[0], sample.Lens[1],
					 ray.Origin, ray.Direction);

		ray.MinT = mNearT;
		ray.MaxT = mFarT;

		return ray;
	}

	inline void constructRay(float nx, float ny,
							 float r1, float r2,
							 Vector3f& o, Vector3f& d) const
	{
		o = transform().translation();
		d = mRight_Cache * nx + mUp_Cache * ny + mFocalDistance_Cache;

		if constexpr (HasDOF) {
			const float t = 2 * PR_PI * r1;
			float s		  = std::sin(t);
			float c		  = std::cos(t);

			const Vector3f e = mXApertureRadius_Cache * r2 * s + mYApertureRadius_Cache * r2 * c;

			o += e;
			d -= e;
		} else {
			PR_UNUSED(r1);
			PR_UNUSED(r2);
		}

		d.normalize();
	}

	void cache()
	{
		// Apply transformation (with scale support)
		mDirection_Cache = transform().linear() * mLocalDirection;
		mRight_Cache	 = transform().linear() * mLocalRight;
		mUp_Cache		 = transform().linear() * mLocalUp;

		PR_LOG(L_DEBUG) << name() << ": Dir" << PR_FMT_MAT(mDirection_Cache)
						<< " Right" << PR_FMT_MAT(mRight_Cache)
						<< " Up" << PR_FMT_MAT(mUp_Cache) << std::endl;

		// No depth of field
		if constexpr (!HasDOF) {
			mFocalDistance_Cache   = mDirection_Cache;
			mXApertureRadius_Cache = Vector3f(0, 0, 0);
			mYApertureRadius_Cache = Vector3f(0, 0, 0);
			mRight_Cache *= 0.5f * mWidth;
			mUp_Cache *= 0.5f * mHeight;
		} else {
			mFocalDistance_Cache   = mDirection_Cache * (mFStop + 1);
			mXApertureRadius_Cache = mRight_Cache * mApertureRadius;
			mYApertureRadius_Cache = mUp_Cache * mApertureRadius;
			mRight_Cache *= 0.5f * mWidth * (mFStop + 1);
			mUp_Cache *= 0.5f * mHeight * (mFStop + 1);

			PR_LOG(L_DEBUG) << "    FocalDistance" << PR_FMT_MAT(mFocalDistance_Cache)
							<< " XAperature" << PR_FMT_MAT(mXApertureRadius_Cache)
							<< " YAperature" << PR_FMT_MAT(mYApertureRadius_Cache) << std::endl;
		}
	}

private:
	const float mWidth;
	const float mHeight;

	const float mFStop;
	const float mApertureRadius;

	const float mNearT;
	const float mFarT;

	const Vector3f mLocalDirection;
	const Vector3f mLocalRight;
	const Vector3f mLocalUp;

	// Cache:
	Vector3f mDirection_Cache;
	Vector3f mRight_Cache;
	Vector3f mUp_Cache;

	Vector3f mFocalDistance_Cache;
	Vector3f mXApertureRadius_Cache;
	Vector3f mYApertureRadius_Cache;
};

class PerspectiveCameraPlugin : public ICameraPlugin {
public:
	std::shared_ptr<ICamera> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();
		std::string name			 = params.getString("name", "__unnamed__");

		const float apr	  = params.getNumber("aperture_radius", 0.05f);
		const float fstop = params.getNumber("fstop", 0);
		const Vector3f ld = params.getVector3f("local_direction", ICamera::DefaultDirection);
		const Vector3f lr = params.getVector3f("local_right", ICamera::DefaultRight);
		const Vector3f lu = params.getVector3f("local_up", ICamera::DefaultUp);

		const float w  = params.getNumber("width", 1);
		const float h  = params.getNumber("height", 1);
		const float nT = params.getNumber("near", NEAR_DEFAULT);
		const float fT = params.getNumber("far", FAR_DEFAULT);

		const bool hasDOF = apr > PR_EPSILON && fstop > PR_EPSILON;

		if (hasDOF)
			return std::make_shared<PerspectiveCamera<true>>(name, ctx.transform(), w, h, fstop, apr, nT, fT, ld, lr, lu);
		else
			return std::make_shared<PerspectiveCamera<false>>(name, ctx.transform(), w, h, fstop, apr, nT, fT, ld, lr, lu);
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "standard_camera", "standard", "default", "perspective" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Perspective Camera", "Camera based on the perspective projection")
			.Identifiers(getNames())
			.Inputs()
			.Number("width", "Width of the image plane", 1)
			.Number("height", "Height of the image plane", 1)
			.Number("near", "Near", NEAR_DEFAULT)
			.Number("far", "Far", FAR_DEFAULT)
			.BeginBlock("DOF")
			.Number("aperture_radius", "Aperature radius in world coordinates", 0.05f)
			.Number("fstop", "Focal stop", 0)
			.EndBlock()
			.Vector("local_direction", "Local view direction", ICamera::DefaultDirection)
			.Vector("local_right", "Local right direction", ICamera::DefaultRight)
			.Vector("local_up", "Local up direction", ICamera::DefaultUp)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::PerspectiveCameraPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)