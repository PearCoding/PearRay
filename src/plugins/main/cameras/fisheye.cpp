#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "camera/ICamera.h"
#include "camera/ICameraPlugin.h"
#include "math/Spherical.h"
#include "math/Tangent.h"

#include "ray/Ray.h"
#include "renderer/RenderContext.h"

namespace PR {
constexpr float NEAR_DEFAULT = 0.000001f;
constexpr float FAR_DEFAULT	 = PR_INF;

// See https://en.wikipedia.org/wiki/Fisheye_lens
enum class MapType {
	Circular = 0,
	Cropped,
	Full
};

// A fisheye camera with equirectangular mapping
// Therefore it is just a sophisticated spherical camera with fix phi and parametric theta
template <bool ClipRange>
class FisheyeCamera : public ICamera {
public:
	ENTITY_CLASS

	FisheyeCamera(const std::string& name, const Transformf& transform,
				  float fov, float nearT, float farT,
				  const Vector3f& ld, const Vector3f& lr, const Vector3f& lu,
				  MapType mapType)
		: ICamera(name, transform)
		, mFOV(fov)
		, mNearT(nearT)
		, mFarT(farT)
		, mLocalDirection(ld)
		, mLocalRight(lr)
		, mLocalUp(lu)
		, mMapType(mapType)
		, mDirection_Cache(transform.linear() * mLocalDirection)
		, mRight_Cache(transform.linear() * mLocalRight)
		, mUp_Cache(transform.linear() * mLocalUp)
	{
		PR_LOG(L_DEBUG) << name << ": Dir" << PR_FMT_MAT(mDirection_Cache)
						<< " Right" << PR_FMT_MAT(mRight_Cache)
						<< " Up" << PR_FMT_MAT(mUp_Cache) << std::endl;
	}

	virtual ~FisheyeCamera()
	{
	}

	std::string type() const override
	{
		return "fisheye";
	}

	std::optional<CameraRay> constructRay(const CameraSample& sample) const override
	{
		const float aspect = sample.SensorSize.Width / (float)sample.SensorSize.Height;
		float xaspect;
		float yaspect;

		switch (mMapType) {
		default:
		case MapType::Circular:
			xaspect = aspect < 1 ? 1 : aspect;
			yaspect = aspect > 1 ? 1 : aspect;
			break;
		case MapType::Cropped:
			xaspect = aspect < 1 ? 1 / aspect : 1;
			yaspect = aspect > 1 ? 1 / aspect : 1;
			break;
		case MapType::Full: {
			const float diameter = std::sqrt(aspect * aspect + 1.0f) * sample.SensorSize.Height;
			const float k		 = std::min<float>(sample.SensorSize.Width, sample.SensorSize.Height);
			const float f		 = diameter / k;

			xaspect = aspect < 1 ? 1 : 1 / aspect;
			yaspect = aspect > 1 ? 1 : aspect;
			xaspect *= f;
			yaspect *= f;
		} break;
		}

		const float nx = 2 * (sample.Pixel[0] / sample.SensorSize.Width - 0.5f) / xaspect;
		const float ny = 2 * (sample.Pixel[1] / sample.SensorSize.Height - 0.5f) / yaspect;

		if constexpr (ClipRange) {
			if (nx * nx + ny * ny > 1)
				return std::optional<CameraRay>();
		}

		CameraRay ray;
		constructRay(nx, -ny, ray.Origin, ray.Direction);

		ray.MinT = mNearT;
		ray.MaxT = mFarT;

		return ray;
	}

	inline void constructRay(float nx, float ny,
							 Vector3f& o, Vector3f& d) const
	{
		o				  = transform().translation();
		const float r	  = std::sqrt(nx * nx + ny * ny);
		const float theta = r * mFOV / 2;

		const float sT = std::sin(theta);
		const float cT = std::cos(theta);
		const float sP = r < PR_EPSILON ? 0 : ny / r;
		const float cP = r < PR_EPSILON ? 0 : nx / r;
		d			   = Tangent::fromTangentSpace(mDirection_Cache, mRight_Cache, mUp_Cache,
									   Vector3f(cP * sT, sP * sT, cT))
				.normalized();
	}

private:
	const float mFOV;

	const float mNearT;
	const float mFarT;

	const Vector3f mLocalDirection;
	const Vector3f mLocalRight;
	const Vector3f mLocalUp;

	const MapType mMapType;

	// Cache:
	const Vector3f mDirection_Cache;
	const Vector3f mRight_Cache;
	const Vector3f mUp_Cache;
};

template <bool ClipRange>
static inline std::shared_ptr<ICamera> createCamera(const ParameterGroup& params, const Transformf& transform)
{
	std::string name	   = params.getString("name", "__unnamed__");
	std::string mapTypeStr = params.getString("map", "circular");
	std::transform(mapTypeStr.begin(), mapTypeStr.end(), mapTypeStr.begin(), ::tolower);

	MapType mapType = MapType::Circular;
	if (mapTypeStr == "cropped")
		mapType = MapType::Cropped;
	else if (mapTypeStr == "full")
		mapType = MapType::Full;

	return std::make_shared<FisheyeCamera<ClipRange>>(name, transform,
													  params.getNumber("fov", 180.0f * PR_DEG2RAD),
													  params.getNumber("near", NEAR_DEFAULT),
													  params.getNumber("far", FAR_DEFAULT),
													  params.getVector3f("local_direction", ICamera::DefaultDirection),
													  params.getVector3f("local_right", ICamera::DefaultRight),
													  params.getVector3f("local_up", ICamera::DefaultUp),
													  mapType);
}

class FisheyeCameraPlugin : public ICameraPlugin {
public:
	std::shared_ptr<ICamera> create(const std::string&, const SceneLoadContext& ctx) override
	{
		bool clip_range = ctx.parameters().getBool("clip_range", true);
		if (clip_range)
			return createCamera<true>(ctx.parameters(), ctx.transform());
		else
			return createCamera<false>(ctx.parameters(), ctx.transform());
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "fisheye" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Fisheye Camera", "Fishy")
			.Identifiers(getNames())
			.Inputs()
			.Option("map", "Map Method", "circular", { "circular", "cropped", "full" })
			.Number("fov", "Field of view in radians", 180.0f * PR_DEG2RAD)
			.Number("near", "Near", NEAR_DEFAULT)
			.Number("far", "Far", FAR_DEFAULT)
			.Vector("local_direction", "Local view direction", ICamera::DefaultDirection)
			.Vector("local_right", "Local right direction", ICamera::DefaultRight)
			.Vector("local_up", "Local up direction", ICamera::DefaultUp)
			.Bool("clip_range", "Filter out rays not inside the range", true)
			.Specification()
			.get();
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::FisheyeCameraPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)