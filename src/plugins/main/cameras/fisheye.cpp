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
enum MapType {
	MT_Circular = 0,
	MT_Cropped,
	MT_Full
};

// A fisheye camera with equirectangular mapping
// Therefore it is just a sophisticated spherical camera with fix phi and parametric theta
template <bool ClipRange>
class FisheyeCamera : public ICamera {
public:
	ENTITY_CLASS

	FisheyeCamera(uint32 id, const std::string& name, const Transformf& transform,
				  float fov, float nearT, float farT,
				  const Vector3f& ld, const Vector3f& lr, const Vector3f& lu,
				  MapType mapType)
		: ICamera(id, name, transform)
		, mFOV(fov)
		, mNearT(nearT)
		, mFarT(farT)
		, mLocalDirection(ld)
		, mLocalRight(lr)
		, mLocalUp(lu)
		, mMapType(mapType)
		, mDirection_Cache((normalMatrix() * mLocalDirection).normalized())
		, mRight_Cache((normalMatrix() * mLocalRight).normalized())
		, mUp_Cache((normalMatrix() * mLocalUp).normalized())
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
		case MT_Circular:
			xaspect = aspect < 1 ? 1 : aspect;
			yaspect = aspect > 1 ? 1 : aspect;
			break;
		case MT_Cropped:
			xaspect = aspect < 1 ? 1 / aspect : 1;
			yaspect = aspect > 1 ? 1 / aspect : 1;
			break;
		case MT_Full: {
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
									   Vector3f(cP * sT, sP * sT, cT));
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
static inline std::shared_ptr<ICamera> createCamera(uint32 id, const ParameterGroup& params, const Transformf& transform)
{
	std::string name	   = params.getString("name", "__unnamed__");
	std::string mapTypeStr = params.getString("map", "circular");
	std::transform(mapTypeStr.begin(), mapTypeStr.end(), mapTypeStr.begin(), ::tolower);

	MapType mapType = MT_Circular;
	if (mapTypeStr == "cropped")
		mapType = MT_Cropped;
	else if (mapTypeStr == "full")
		mapType = MT_Full;

	return std::make_shared<FisheyeCamera<ClipRange>>(id, name, transform,
													  params.getNumber("fov", 180.0f * PR_DEG2RAD),
													  params.getNumber("near", NEAR_DEFAULT),
													  params.getNumber("far", FAR_DEFAULT),
													  params.getVector3f("localDirection", ICamera::DefaultDirection),
													  params.getVector3f("localRight", ICamera::DefaultRight),
													  params.getVector3f("localUp", ICamera::DefaultUp),
													  mapType);
}

class FisheyeCameraPlugin : public ICameraPlugin {
public:
	std::shared_ptr<ICamera> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		bool clip_range = ctx.parameters().getBool("clip_range", true);
		if (clip_range)
			return createCamera<true>(id, ctx.parameters(), ctx.transform());
		else
			return createCamera<false>(id, ctx.parameters(), ctx.transform());
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "fisheye" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::FisheyeCameraPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)