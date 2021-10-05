#include "Scene.h"
#include "Platform.h"
#include "SceneDatabase.h"
#include "ServiceObserver.h"
#include "camera/ICamera.h"
#include "entity/GeometryDev.h"
#include "entity/GeometryRepr.h"
#include "entity/IEntity.h"
#include "geometry/GeometryPoint.h"
#include "ray/RayStream.h"
#include "trace/HitStream.h"

#include "Logger.h"

namespace PR {
Scene::Scene(const std::shared_ptr<ServiceObserver>& serviceObserver,
			 const std::shared_ptr<ICamera>& activeCamera,
			 const std::shared_ptr<SceneDatabase>& database)
	: mServiceObserver(serviceObserver)
	, mActiveCamera(activeCamera)
	, mDatabase(database)
{
	PR_LOG(L_DEBUG) << "Setup before scene build..." << std::endl;
	mServiceObserver->callBeforeSceneBuild();
	setupScene();
	PR_LOG(L_DEBUG) << "Setup after scene build..." << std::endl;
	mServiceObserver->callAfterSceneBuild(this);
}

Scene::~Scene()
{
}

void Scene::beforeRender(RenderContext* ctx)
{
	PR_LOG(L_DEBUG) << "Setup before render start..." << std::endl;
	mServiceObserver->callBeforeRender(ctx);
}

void Scene::afterRender(RenderContext* ctx)
{
	PR_LOG(L_DEBUG) << "Setup after render stop..." << std::endl;
	mServiceObserver->callAfterRender(ctx);
}

IEntity* Scene::getEntity(uint32 id) const { return database()->Entities->getSafe(id).get(); }
IMaterial* Scene::getMaterial(uint32 id) const { return database()->Materials->getSafe(id).get(); }
IEmission* Scene::getEmission(uint32 id) const { return database()->Emissions->getSafe(id).get(); }

static void embree_error_function(void* /*userPtr*/, RTCError /*code*/, const char* str)
{
	PR_LOG(L_ERROR) << "[Embree3] " << str << std::endl;
}

struct SceneInternal {
	RTCDevice Device;
	RTCScene Scene;

	inline SceneInternal()
#if defined(PR_DEBUG)
		: Device(rtcNewDevice("verbose=1"))
#else
		: Device(rtcNewDevice(nullptr))
#endif
	{
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

	const auto& entities = mDatabase->Entities->getAll();
	for (size_t i = 0; i < entities.size(); ++i) {
		auto repr = entities[i]->constructGeometryRepresentation(GeometryDev(mInternal->Device));
		rtcAttachGeometryByID(mInternal->Scene, repr, i);
		rtcReleaseGeometry(repr); // No longer needed
	}

	rtcSetSceneFlags(mInternal->Scene, RTC_SCENE_FLAG_COMPACT | RTC_SCENE_FLAG_ROBUST);
	rtcSetSceneBuildQuality(mInternal->Scene, RTC_BUILD_QUALITY_HIGH);
	rtcCommitScene(mInternal->Scene);

	if (rtcGetDeviceError(mInternal->Device) != RTC_ERROR_NONE)
		throw std::runtime_error("Could not build scene");

	// Extract scene boundary
	mBoundingBox	= BoundingBox();
	mBoundingSphere = Sphere();

	if (!entities.empty()) {
		RTCBounds bounds;
		rtcGetSceneBounds(mInternal->Scene, &bounds);
		mBoundingBox = BoundingBox(Vector3f(bounds.lower_x, bounds.lower_y, bounds.lower_z),
								   Vector3f(bounds.upper_x, bounds.upper_y, bounds.upper_z));

		// Extrace origin centered bounding sphere
		mBoundingSphere.combine(mBoundingBox.upperBound());
		mBoundingSphere.combine(mBoundingBox.lowerBound());
	}
}

constexpr unsigned int MASK_ALL = 0xFFFFFFF;
constexpr int VALID_ALL			= -1; //0xFF;
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

	alignas(64) int valids[PACKAGE_SIZE];
	std::fill_n(valids, PACKAGE_SIZE, VALID_ALL); // All are valid

	while (rays.hasNextSpan()) {
		RaySpan grp = rays.getNextSpan();

		const RTCIntersectContextFlags flags = grp.isCoherent() ? RTC_INTERSECT_CONTEXT_FLAG_COHERENT : RTC_INTERSECT_CONTEXT_FLAG_INCOHERENT;

		// Split group into cells of max 16 rays each
		const size_t cells = grp.size() / PACKAGE_SIZE;
		for (size_t i = 0; i < cells; ++i) {
			PR_OPT_LOOP
			for (size_t k = 0; k < PACKAGE_SIZE; ++k) {
				rhits.hit.geomID[k]	   = RTC_INVALID_GEOMETRY_ID;
				rhits.hit.instID[0][k] = RTC_INVALID_GEOMETRY_ID;
				rhits.ray.flags[k]	   = 0;
				rhits.ray.id[k]		   = k;
				rhits.ray.mask[k]	   = MASK_ALL;
			}

			grp.copyOriginXRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.org_x);
			grp.copyOriginYRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.org_y);
			grp.copyOriginZRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.org_z);
			grp.copyDirectionXRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.dir_x);
			grp.copyDirectionYRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.dir_y);
			grp.copyDirectionZRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.dir_z);
			grp.copyMaxTRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.tfar);
			grp.copyMinTRaw(i * PACKAGE_SIZE, PACKAGE_SIZE, rhits.ray.tnear);

			rtcInitIntersectContext(&ctx);
			ctx.flags = flags;
			rtcIntersect16(valids, scene, &ctx, &rhits);

			PR_OPT_LOOP
			for (size_t k = 0; k < PACKAGE_SIZE; ++k) {
				const bool good = rhits.hit.geomID[k] != RTC_INVALID_GEOMETRY_ID;
				const auto id	= rhits.hit.instID[0][k] != RTC_INVALID_GEOMETRY_ID ? rhits.hit.instID[0][k] : rhits.hit.geomID[k];

				HitEntry entry;
				entry.EntityID	  = good ? id : PR_INVALID_ID;
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
			rhit.hit.geomID	   = RTC_INVALID_GEOMETRY_ID;
			rhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

			rtcInitIntersectContext(&ctx);
			ctx.flags = flags;
			rtcIntersect1(mInternal->Scene, &ctx, &rhit);

			const bool good = rhit.hit.geomID != RTC_INVALID_GEOMETRY_ID;
			const auto id	= rhit.hit.instID[0] != RTC_INVALID_GEOMETRY_ID ? rhit.hit.instID[0] : rhit.hit.geomID;

			HitEntry entry;
			entry.EntityID	  = good ? id : PR_INVALID_ID;
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
	rhit.hit.geomID	   = RTC_INVALID_GEOMETRY_ID;
	rhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

	rtcIntersect1(mInternal->Scene, &ctx, &rhit);

	if (rhit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
		return false;
	} else {
		entry.EntityID	  = rhit.hit.instID[0] != RTC_INVALID_GEOMETRY_ID ? rhit.hit.instID[0] : rhit.hit.geomID;
		entry.PrimitiveID = rhit.hit.primID;
		entry.RayID		  = 0;
		entry.Parameter	  = Vector3f(rhit.hit.u, rhit.hit.v, rhit.ray.tfar);
		return true;
	}
}

bool Scene::traceSingleRay(const Ray& ray, Vector3f& pos, GeometryPoint& pt) const
{
	HitEntry entry;
	if (!traceSingleRay(ray, entry))
		return false;

	const auto entity = getEntity(entry.EntityID);
	if (!entity)
		return false;

	EntityGeometryQueryPoint query;
	query.PrimitiveID = entry.PrimitiveID;
	query.UV		  = Vector2f(entry.Parameter[0], entry.Parameter[1]);
	query.View		  = ray.Direction;
	query.Position	  = ray.t(entry.Parameter.z());

	entity->provideGeometryPoint(query, pt);
	pos = query.Position;

	return true;
}

bool Scene::traceShadowRay(const Ray& ray, float distance) const
{
	RTCIntersectContext ctx;
	rtcInitIntersectContext(&ctx);
	ctx.flags = RTC_INTERSECT_CONTEXT_FLAG_INCOHERENT;

	RTCRay rray;
	assignRay(ray, rray);

	rray.tfar = distance - 0.001f; // FIXME: What is the perfect value for that??

	rtcOccluded1(mInternal->Scene, &ctx, &rray);

	return rray.tfar == -PR_INF; // RTCRay.tfar is set to -inf if hit anything
}

size_t Scene::entityCount() const { return mDatabase->Entities->size(); }
size_t Scene::emissionCount() const { return mDatabase->Emissions->size(); }
size_t Scene::materialCount() const { return mDatabase->Materials->size(); }
size_t Scene::infiniteLightCount() const { return mDatabase->InfiniteLights->size(); }
size_t Scene::nodeCount() const { return mDatabase->Nodes->size(); }

} // namespace PR
