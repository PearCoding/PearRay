#include "Scene.h"
#include "Platform.h"
#include "camera/ICamera.h"
#include "emission/IEmission.h"
#include "entity/GeometryDev.h"
#include "entity/GeometryRepr.h"
#include "infinitelight/IInfiniteLight.h"
#include "material/IMaterial.h"
#include "renderer/RenderContext.h"
#include "serialization/FileSerializer.h"

#include "Logger.h"

#include <boost/filesystem.hpp>
#include <fstream>

namespace PR {
Scene::Scene(const std::shared_ptr<ICamera>& activeCamera,
			 const std::vector<std::shared_ptr<IEntity>>& entities,
			 const std::vector<std::shared_ptr<IMaterial>>& materials,
			 const std::vector<std::shared_ptr<IEmission>>& emissions,
			 const std::vector<std::shared_ptr<IInfiniteLight>>& infLights)
	: mActiveCamera(activeCamera)
	, mEntities(entities)
	, mMaterials(materials)
	, mEmissions(emissions)
	, mInfLights(infLights)
{
	PR_LOG(L_DEBUG) << "Setup before scene build..." << std::endl;
	mActiveCamera->beforeSceneBuild();
	for (auto o : mEmissions)
		o->beforeSceneBuild();
	for (auto o : mInfLights)
		o->beforeSceneBuild();
	for (auto o : mMaterials)
		o->beforeSceneBuild();
	for (auto o : mEntities)
		o->beforeSceneBuild();

	setupScene();

	PR_LOG(L_DEBUG) << "Setup after scene build..." << std::endl;
	mActiveCamera->afterSceneBuild(this);
	for (auto o : mEmissions)
		o->afterSceneBuild(this);
	for (auto o : mInfLights)
		o->afterSceneBuild(this);
	for (auto o : mMaterials)
		o->afterSceneBuild(this);
	for (auto o : mEntities)
		o->afterSceneBuild(this);
}

Scene::~Scene()
{
}
void Scene::beforeRender(RenderContext* ctx)
{
	PR_LOG(L_DEBUG) << "Setup before render start..." << std::endl;

	mActiveCamera->beforeRender(ctx);
	for (auto o : mEmissions)
		o->beforeRender(ctx);
	for (auto o : mInfLights)
		o->beforeRender(ctx);
	for (auto o : mMaterials)
		o->beforeRender(ctx);
	for (auto o : mEntities)
		o->beforeRender(ctx);

	mDeltaInfLights.clear();
	mNonDeltaInfLights.clear();
	for (auto o : mInfLights) {
		if (o->hasDeltaDistribution())
			mDeltaInfLights.push_back(o);
		else
			mNonDeltaInfLights.push_back(o);
	}
}

void Scene::afterRender(RenderContext* ctx)
{
	PR_LOG(L_DEBUG) << "Setup after render stop..." << std::endl;

	mActiveCamera->afterRender(ctx);
	for (auto o : mEmissions)
		o->afterRender(ctx);
	for (auto o : mInfLights)
		o->afterRender(ctx);
	for (auto o : mMaterials)
		o->afterRender(ctx);
	for (auto o : mEntities)
		o->afterRender(ctx);
}

static void embree_error_function(void* /*userPtr*/, RTCError /*code*/, const char* str)
{
	PR_LOG(L_ERROR) << "[Embree3] " << str << std::endl;
}

struct SceneInternal {
	RTCDevice Device;
	RTCScene Scene;

	inline SceneInternal()
	{
#if 1 //defined(PR_DEBUG)
		Device = rtcNewDevice("verbose=1");
#else
		Device = rtcNewDevice(nullptr);
#endif
		rtcSetDeviceErrorFunction(Device, embree_error_function, nullptr);

		if (rtcGetDeviceProperty(Device, RTC_DEVICE_PROPERTY_BACKFACE_CULLING_ENABLED))
			PR_LOG(L_WARNING) << "[Embree3] Backface culling is enabled. PearRay may behave strange" << std::endl;

		if (!rtcGetDeviceProperty(Device, RTC_DEVICE_PROPERTY_TRIANGLE_GEOMETRY_SUPPORTED))
			PR_LOG(L_ERROR) << "[Embree3] Embree without triangle support is not suitable for PearRay" << std::endl;
		if (!rtcGetDeviceProperty(Device, RTC_DEVICE_PROPERTY_POINT_GEOMETRY_SUPPORTED))
			PR_LOG(L_ERROR) << "[Embree3] Embree without point geometry (like sphere) support is not suitable for PearRay" << std::endl;
		if (!rtcGetDeviceProperty(Device, RTC_DEVICE_PROPERTY_USER_GEOMETRY_SUPPORTED))
			PR_LOG(L_ERROR) << "[Embree3] Embree without user geometry support is not suitable for PearRay" << std::endl;

		Scene = rtcNewScene(Device);
	}

	inline ~SceneInternal()
	{
		rtcReleaseScene(Scene);
		rtcReleaseDevice(Device);
	}
};

void Scene::setupScene()
{
	mInternal = std::make_unique<SceneInternal>();

	for (auto o : mEntities) {
		auto repr = o->constructGeometryRepresentation(GeometryDev(mInternal->Device));
		rtcAttachGeometryByID(mInternal->Scene, repr, o->id());
		rtcReleaseGeometry(repr); // No longer needed
	}

	rtcSetSceneFlags(mInternal->Scene, RTC_SCENE_FLAG_COMPACT | RTC_SCENE_FLAG_ROBUST);
	rtcSetSceneBuildQuality(mInternal->Scene, RTC_BUILD_QUALITY_HIGH);
	rtcCommitScene(mInternal->Scene);

	if (rtcGetDeviceError(mInternal->Device) != RTC_ERROR_NONE) {
		throw std::runtime_error("Could not build scene");
	}

	// Extract scene boundary
	RTCBounds bounds;
	rtcGetSceneBounds(mInternal->Scene, &bounds);
	mBoundingBox = BoundingBox(Vector3f(bounds.lower_x, bounds.lower_y, bounds.lower_z),
							   Vector3f(bounds.upper_x, bounds.upper_y, bounds.upper_z));
}

constexpr unsigned int MASK_ALL = 0xFFFFFFF;
constexpr int VALID_ALL			= 0xFF;
inline static void assignRay(const Ray& ray, RTCRay& rray)
{
	rray.org_x = ray.Origin[0];
	rray.org_y = ray.Origin[1];
	rray.org_z = ray.Origin[2];
	rray.dir_x = ray.Direction[0];
	rray.dir_y = ray.Direction[1];
	rray.dir_z = ray.Direction[2];
	rray.flags = 0;
	rray.tfar  = ray.MaxT;
	rray.tnear = ray.MinT;
	rray.time  = ray.Time;
	rray.mask  = MASK_ALL;
}

void Scene::traceRays(RayStream& rays, HitStream& hits) const
{
	constexpr size_t PACKAGE_SIZE = 16;

	// Split stream into specific groups
	hits.reset();

	auto scene = mInternal->Scene;
	RTCIntersectContext ctx;
	RTCRayHit16 rhits;

	PR_ALIGN(64)
	int valids[PACKAGE_SIZE];
	std::fill_n(valids, PACKAGE_SIZE, VALID_ALL); // All are valid

	while (rays.hasNextGroup()) {
		RayGroup grp = rays.getNextGroup();

		rtcInitIntersectContext(&ctx);
		ctx.flags = grp.isCoherent() ? RTC_INTERSECT_CONTEXT_FLAG_COHERENT : RTC_INTERSECT_CONTEXT_FLAG_INCOHERENT;

		// Split group into cells of max 16 rays each
		const size_t cells = grp.size() / PACKAGE_SIZE;
		for (size_t i = 0; i < cells; ++i) {
			PR_OPT_LOOP
			for (size_t k = 0; k < PACKAGE_SIZE; ++k) {
				rhits.hit.geomID[k] = RTC_INVALID_GEOMETRY_ID;
				rhits.ray.flags[k]	= 0;
				rhits.ray.id[k]		= k;
				rhits.ray.mask[k]	= MASK_ALL;
			}

			grp.copyOriginXRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.org_x);
			grp.copyOriginYRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.org_y);
			grp.copyOriginZRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.org_z);
			grp.copyDirectionXRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.dir_x);
			grp.copyDirectionYRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.dir_y);
			grp.copyDirectionZRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.dir_z);
			grp.copyMaxTRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.tfar);
			grp.copyMinTRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.tnear);
			grp.copyTimeRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.time);

			rtcIntersect16(valids, scene, &ctx, &rhits);

			PR_OPT_LOOP
			for (size_t k = 0; k < PACKAGE_SIZE; ++k) {
				const bool good = rhits.hit.geomID[k] != RTC_INVALID_GEOMETRY_ID;

				HitEntry entry;
				entry.Flags		  = good ? HF_SUCCESSFUL : 0;
				entry.EntityID	  = good ? rhits.hit.geomID[k] : PR_INVALID_ID;
				entry.PrimitiveID = rhits.hit.primID[k];
				entry.RayID		  = grp.offset() + i * PACKAGE_SIZE + rhits.ray.id[k];
				entry.Parameter	  = Vector3f(rhits.hit.u[k], rhits.hit.v[k], rhits.ray.tfar[k]);
				hits.add(entry);
			}
		}

		// Trace remaining rays which did not fit into a 16 package cell
		for (size_t k = cells * PACKAGE_SIZE; k < grp.size(); ++k) {
			RTCRayHit rhit;
			assignRay(grp.getRay(k), rhit.ray);
			rhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

			rtcIntersect1(mInternal->Scene, &ctx, &rhit);

			const bool good = rhit.hit.geomID != RTC_INVALID_GEOMETRY_ID;
			HitEntry entry;
			entry.Flags		  = good ? HF_SUCCESSFUL : 0;
			entry.EntityID	  = good ? rhit.hit.geomID : PR_INVALID_ID;
			entry.PrimitiveID = rhit.hit.primID;
			entry.RayID		  = grp.offset() + k;
			entry.Parameter	  = Vector3f(rhit.hit.u, rhit.hit.v, rhit.ray.tfar);
			hits.add(entry);
		}
	}
}

bool Scene::traceSingleRay(const Ray& ray, HitEntry& entry) const
{
	RTCIntersectContext ctx;
	rtcInitIntersectContext(&ctx);
	ctx.flags = RTC_INTERSECT_CONTEXT_FLAG_INCOHERENT;

	RTCRayHit rhit;
	assignRay(ray, rhit.ray);
	rhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

	rtcIntersect1(mInternal->Scene, &ctx, &rhit);

	if (rhit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
		return false;
	} else {
		entry.EntityID	  = rhit.hit.geomID;
		entry.PrimitiveID = rhit.hit.primID;
		entry.RayID		  = 0;
		entry.Flags		  = HF_SUCCESSFUL;
		entry.Parameter	  = Vector3f(rhit.hit.u, rhit.hit.v, rhit.ray.tfar);
		return true;
	}
}

bool Scene::traceShadowRay(const Ray& ray, float distance, uint32 entity_id) const
{
	// TODO: Better use a mask to filter out entity and check for any hits
	PR_UNUSED(entity_id);

	RTCIntersectContext ctx;
	rtcInitIntersectContext(&ctx);
	ctx.flags = RTC_INTERSECT_CONTEXT_FLAG_INCOHERENT;

	RTCRay rray;
	assignRay(ray, rray);
	rray.tfar = distance - 0.0001f;

	rtcOccluded1(mInternal->Scene, &ctx, &rray);

	return rray.tfar != -std::numeric_limits<float>::infinity();
}

bool Scene::traceOcclusionRay(const Ray& ray) const
{
	RTCIntersectContext ctx;
	rtcInitIntersectContext(&ctx);
	ctx.flags = RTC_INTERSECT_CONTEXT_FLAG_INCOHERENT;

	RTCRay rray;
	assignRay(ray, rray);

	rtcOccluded1(mInternal->Scene, &ctx, &rray);

	return rray.tfar == -std::numeric_limits<float>::infinity(); // RTCRay.tfar is set to -inf if hit anything
}
} // namespace PR
