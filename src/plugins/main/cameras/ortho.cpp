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

class OrthoCamera : public ICamera {
public:
	ENTITY_CLASS

	OrthoCamera(const std::string& name, const Transformf& transform,
				float w, float h, float nearT, float farT,
				const Vector3f& ld, const Vector3f& lr, const Vector3f& lu)
		: ICamera(name, transform)
		, mWidth(w)
		, mHeight(h)
		, mNearT(nearT)
		, mFarT(farT)
		, mLocalDirection(ld)
		, mLocalRight(lr)
		, mLocalUp(lu)
		, mDirection_Cache((transform.linear() * mLocalDirection).normalized())
		, mRight_Cache(transform.linear() * mLocalRight * 0.5f * mWidth)
		, mUp_Cache(transform.linear() * mLocalUp * 0.5f * mHeight)
	{
		PR_LOG(L_DEBUG) << name << ": Dir" << PR_FMT_MAT(mDirection_Cache)
						<< " Right" << PR_FMT_MAT(mRight_Cache)
						<< " Up" << PR_FMT_MAT(mUp_Cache) << std::endl;
	}

	virtual ~OrthoCamera()
	{
	}

	std::string type() const override
	{
		return "orthographic";
	}

	std::optional<CameraRay> constructRay(const CameraSample& sample) const override
	{
		const float nx = 2 * (sample.Pixel[0] / sample.SensorSize.Width - 0.5f);
		const float ny = 2 * (sample.Pixel[1] / sample.SensorSize.Height - 0.5f);

		CameraRay ray;
		constructRay(nx, -ny, ray.Origin, ray.Direction);

		ray.MinT = mNearT;
		ray.MaxT = mFarT;

		return ray;
	}

	inline void constructRay(float nx, float ny,
							 Vector3f& o, Vector3f& d) const
	{
		o = transform().translation() + mRight_Cache * nx + mUp_Cache * ny;
		d = mDirection_Cache;
	}

private:
	const float mWidth;
	const float mHeight;

	const float mNearT;
	const float mFarT;

	const Vector3f mLocalDirection;
	const Vector3f mLocalRight;
	const Vector3f mLocalUp;

	const Vector3f mDirection_Cache;
	const Vector3f mRight_Cache;
	const Vector3f mUp_Cache;
};

class OrthoCameraPlugin : public ICameraPlugin {
public:
	std::shared_ptr<ICamera> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();
		std::string name			 = params.getString("name", "__unnamed__");

		return std::make_shared<OrthoCamera>(name, ctx.transform(),
											 params.getNumber("width", 1),
											 params.getNumber("height", 1),
											 params.getNumber("near", NEAR_DEFAULT),
											 params.getNumber("far", FAR_DEFAULT),
											 params.getVector3f("local_direction", ICamera::DefaultDirection),
											 params.getVector3f("local_right", ICamera::DefaultRight),
											 params.getVector3f("local_up", ICamera::DefaultUp));
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "ortho", "orthographic" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Orthogonal Camera", "Camera based on the orthographic projection")
			.Identifiers(getNames())
			.Inputs()
			.Number("width", "Width of the image plane", 1)
			.Number("height", "Height of the image plane", 1)
			.Number("near", "Near", NEAR_DEFAULT)
			.Number("far", "Far", FAR_DEFAULT)
			.Vector("local_direction", "Local view direction", ICamera::DefaultDirection)
			.Vector("local_right", "Local right direction", ICamera::DefaultRight)
			.Vector("local_up", "Local up direction", ICamera::DefaultUp)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::OrthoCameraPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)