#include "geometry/Quadric.h"
#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "emission/IEmission.h"
#include "entity/GeometryDev.h"
#include "entity/GeometryRepr.h"
#include "entity/IEntity.h"
#include "entity/IEntityPlugin.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Tangent.h"

#include <array>
namespace PR {

static void userBoundsFunc(const RTCBoundsFunctionArguments* args);
static void userIntersectFuncN(const RTCIntersectFunctionNArguments* args);
static void userOccludedFuncN(const RTCOccludedFunctionNArguments* args);

class QuadricEntity : public IEntity {
public:
	ENTITY_CLASS

	QuadricEntity(const std::string& name, const Transformf& transform,
				  const std::array<float, 10>& parameters,
				  const Vector3f& minB, const Vector3f& maxB,
				  uint32 matID, uint32 lightID)
		: IEntity(lightID, name, transform)
		, mBoundingBox(minB, maxB)
		, mWorldBoundingBox(worldBoundingBox())
		, mParameters(parameters)
		, mMaterialID(matID)
	{
	}
	virtual ~QuadricEntity() {}

	std::string type() const override
	{
		return "quadric";
	}

	float localSurfaceArea(uint32 /*id*/) const override
	{
		// TODO: Fix this and get an accurate surface area
		return mBoundingBox.surfaceArea();
	}

	bool isCollidable() const override
	{
		return mBoundingBox.isValid();
	}

	float collisionCost() const override
	{
		return 10;
	}

	BoundingBox localBoundingBox() const override
	{
		return mBoundingBox;
	}

	GeometryRepr constructGeometryRepresentation(const GeometryDev& dev) const override
	{
		RTCGeometry geom = rtcNewGeometry(dev, RTC_GEOMETRY_TYPE_USER);
		rtcSetGeometryUserPrimitiveCount(geom, 1);
		rtcSetGeometryUserData(geom, const_cast<QuadricEntity*>(this));
		rtcSetGeometryBoundsFunction(geom, userBoundsFunc, nullptr);
		rtcSetGeometryIntersectFunction(geom, userIntersectFuncN);
		rtcSetGeometryOccludedFunction(geom, userOccludedFuncN);

		rtcCommitGeometry(geom);

		return GeometryRepr(geom);
	}

	// TODO
	EntitySamplePoint sampleParameterPoint(const Vector2f&) const override
	{
		return EntitySamplePoint(transform() * Vector3f(0, 0, 0), Vector2f::Zero(), 0, 0);
	}

	// TODO
	float sampleParameterPointPDF() const override { return 0; }

	void provideGeometryPoint(const EntityGeometryQueryPoint& query,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		pt.N = normalMatrix() * Quadric::normal(mParameters, invTransform() * query.Position);
		Tangent::frame(pt.N, pt.Nx, pt.Ny);

		pt.UV		   = query.UV;
		pt.PrimitiveID = 0;
		pt.MaterialID  = mMaterialID;
		pt.EmissionID  = emissionID();
		pt.DisplaceID  = 0;
	}

	inline const BoundingBox& wbbox() const { return mWorldBoundingBox; }
	inline const std::array<float, 10>& parameters() const { return mParameters; }

private:
	BoundingBox mBoundingBox;
	BoundingBox mWorldBoundingBox;
	std::array<float, 10> mParameters;

	const uint32 mMaterialID;
};

static void userBoundsFunc(const RTCBoundsFunctionArguments* args)
{
	const QuadricEntity* entity = (const QuadricEntity*)args->geometryUserPtr;

	RTCBounds* bounds_o = args->bounds_o;
	bounds_o->lower_x	= entity->wbbox().lowerBound()(0);
	bounds_o->lower_y	= entity->wbbox().lowerBound()(1);
	bounds_o->lower_z	= entity->wbbox().lowerBound()(2);
	bounds_o->upper_x	= entity->wbbox().upperBound()(0);
	bounds_o->upper_y	= entity->wbbox().upperBound()(1);
	bounds_o->upper_z	= entity->wbbox().upperBound()(2);
}

static void userIntersectFuncN(const RTCIntersectFunctionNArguments* args)
{
	/* avoid crashing when debug visualizations are used */
	if (args->context == nullptr)
		return;

	int* valid					= (int*)args->valid;
	const QuadricEntity* entity = (const QuadricEntity*)args->geometryUserPtr;
	unsigned int N				= args->N;
	RTCRayHitN* rayhit			= (RTCRayHitN*)args->rayhit;
	RTCRayN* rays				= RTCRayHitN_RayN(rayhit, N);
	RTCHitN* hits				= RTCRayHitN_HitN(rayhit, N);

	/* iterate over all rays in ray packet */
	for (unsigned int ui = 0; ui < N; ui += 1) {
		/* calculate loop and execution mask */
		unsigned int vi = ui + 0;
		if (vi >= N)
			continue;

		/* ignore inactive rays */
		if (valid[vi] != -1)
			continue;

		const Vector3f ray_org = Vector3f(RTCRayN_org_x(rays, N, ui), RTCRayN_org_y(rays, N, ui), RTCRayN_org_z(rays, N, ui));
		const Vector3f ray_dir = Vector3f(RTCRayN_dir_x(rays, N, ui), RTCRayN_dir_y(rays, N, ui), RTCRayN_dir_z(rays, N, ui));
		float& ray_tnear	   = RTCRayN_tnear(rays, N, ui);
		float& ray_tfar		   = RTCRayN_tfar(rays, N, ui);

		// Map to local coordinates
		const Vector3f local_org = entity->invTransform() * ray_org;
		const Vector3f local_dir = entity->invTransform().linear() * ray_dir;
		const PR::Ray local_ray(local_org, local_dir);

		// Check entry and exit
		auto range = entity->localBoundingBox().intersectsRange(local_ray);
		if (range.Entry < 0)
			range.Entry = 0;

		RTCHit potentialhit;
		potentialhit.u		   = 0.0f;
		potentialhit.v		   = 0.0f;
		potentialhit.instID[0] = args->context->instID[0];
#if (RTC_MAX_INSTANCE_LEVEL_COUNT > 1)
		for (unsigned l = 1; l < RTC_MAX_INSTANCE_LEVEL_COUNT && l < args->context->instStackSize; ++l)
			potentialhit.instID[l] = args->context->instID[l];
#endif
		potentialhit.geomID = args->geomID;
		potentialhit.primID = 0;

		float t;
		if (Quadric::intersect(entity->parameters(), local_ray.t(range.Entry), local_dir, t)) {
			t += range.Entry;
			if (t > range.Exit)
				continue;

			// Extract normal information
			const Vector3f xyz = local_ray.t(t);
			const Vector3f Ng  = entity->normalMatrix() * Quadric::normal(entity->parameters(), xyz);

			potentialhit.Ng_x = Ng(0);
			potentialhit.Ng_y = Ng(1);
			potentialhit.Ng_z = Ng(2);

			// Check if its still valid in global context
			const float global_t = local_ray.transformDistance(t, entity->transform().linear());
			if (global_t >= ray_tnear && global_t <= ray_tfar) {
				ray_tfar = t;
				rtcCopyHitToHitN(hits, &potentialhit, N, ui);
			}
		}
	}
}

static void userOccludedFuncN(const RTCOccludedFunctionNArguments* args)
{
	/* avoid crashing when debug visualizations are used */
	if (args->context == nullptr)
		return;

	int* valid					= (int*)args->valid;
	const QuadricEntity* entity = (const QuadricEntity*)args->geometryUserPtr;
	unsigned int N				= args->N;
	RTCRayN* rays				= (RTCRayN*)args->ray;

	/* iterate over all rays in ray packet */
	for (unsigned int ui = 0; ui < N; ui += 1) {
		/* calculate loop and execution mask */
		unsigned int vi = ui + 0;
		if (vi >= N)
			continue;

		/* ignore inactive rays */
		if (valid[vi] != -1)
			continue;

		const Vector3f ray_org = Vector3f(RTCRayN_org_x(rays, N, ui), RTCRayN_org_y(rays, N, ui), RTCRayN_org_z(rays, N, ui));
		const Vector3f ray_dir = Vector3f(RTCRayN_dir_x(rays, N, ui), RTCRayN_dir_y(rays, N, ui), RTCRayN_dir_z(rays, N, ui));
		float& ray_tfar		   = RTCRayN_tfar(rays, N, ui);

		// Map to local coordinates
		const Vector3f local_org = entity->invTransform() * ray_org;
		const Vector3f local_dir = entity->invTransform().linear() * ray_dir;
		const PR::Ray local_ray(local_org, local_dir);

		// Check entry and exit
		auto range = entity->localBoundingBox().intersectsRange(local_ray);
		if (range.Entry < 0)
			range.Entry = 0;

		float t;
		if (Quadric::intersect(entity->parameters(), local_ray.t(range.Entry), local_dir, t))
			ray_tfar = -std::numeric_limits<float>::infinity();
	}
}

class QuadricEntityPlugin : public IEntityPlugin {
public:
	std::shared_ptr<IEntity> create(const std::string& type_name, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.parameters();

		const std::string name = params.getString("name", "__unnamed__");
		const uint32 matID	   = ctx.lookupMaterialID(params.getParameter("material"));
		const uint32 emsID	   = ctx.lookupEmissionID(params.getParameter("emission"));

		if (type_name == "cylinder") {
			const float radius	 = params.getNumber("radius", 1);
			const float height	 = params.getNumber("height", 1);
			const bool center_on = params.getNumber("center_on", true);

			const float a2 = 1 / (radius * radius);

			if (center_on) {
				const Vector3f minB = Vector3f(-radius, -radius, -height / 2);
				const Vector3f maxB = Vector3f(radius, radius, height / 2);

				return std::make_shared<QuadricEntity>(name, ctx.transform(), std::array<float, 10>{ a2, a2, 0, 0, 0, 0, 0, 0, 0, -1 }, minB, maxB, matID, emsID);
			} else {
				const Vector3f minB = Vector3f(-radius, -radius, 0);
				const Vector3f maxB = Vector3f(radius, radius, height);

				return std::make_shared<QuadricEntity>(name, ctx.transform(), std::array<float, 10>{ a2, a2, 0, 0, 0, 0, 0, 0, 0, -1 }, minB, maxB, matID, emsID);
			}
		} else if (type_name == "cone") {
			const float radius	 = params.getNumber("radius", 1);
			const float height	 = params.getNumber("height", 1);
			const bool center_on = params.getNumber("center_on", true);

			const float a2 = 1 / (radius * radius);
			const float h2 = 1 / (height * height);

			if (center_on) {
				const Vector3f minB = Vector3f(-radius, -radius, -height / 2);
				const Vector3f maxB = Vector3f(radius, radius, height / 2);

				const float kz = 1 / height;

				return std::make_shared<QuadricEntity>(name, ctx.transform(), std::array<float, 10>{ a2, a2, -h2, 0, 0, 0, 0, 0, -kz, -0.25f }, minB, maxB, matID, emsID);
			} else {
				const Vector3f minB = Vector3f(-radius, -radius, 0);
				const Vector3f maxB = Vector3f(radius, radius, height);

				return std::make_shared<QuadricEntity>(name, ctx.transform(), std::array<float, 10>{ a2, a2, -h2, 0, 0, 0, 0, 0, 0, 0 }, minB, maxB, matID, emsID);
			}
		} else {
			std::vector<float> qp = params.getNumberArray("parameters");
			Vector3f minB		  = params.getVector3f("min", Vector3f(-1, -1, -1));
			Vector3f maxB		  = params.getVector3f("max", Vector3f(1, 1, 1));

			std::array<float, 10> parameters;
			if (qp.size() == 3)
				parameters = { qp[0], qp[1], qp[2], 0, 0, 0, 0, 0, 0, 0 };
			else if (qp.size() == 4)
				parameters = { qp[0], qp[1], qp[2], 0, 0, 0, 0, 0, 0, qp[3] };
			else if (qp.size() == 10)
				parameters = { qp[0], qp[1], qp[2], qp[3], qp[4], qp[5], qp[6], qp[7], qp[8], qp[9] };
			else {
				PR_LOG(L_ERROR) << "Invalid quadric parameters given" << std::endl;
				return nullptr;
			}

			return std::make_shared<QuadricEntity>(name, ctx.transform(), parameters, minB, maxB, matID, emsID);
		}
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "quadric", "cone", "cylinder" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::QuadricEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)