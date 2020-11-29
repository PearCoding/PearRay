#include "geometry/Disk.h"
#include "Environment.h"
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

namespace PR {

class DiskEntity : public IEntity {
public:
	ENTITY_CLASS

	DiskEntity(const std::string& name, const Transformf& transform,
			   float radius,
			   int32 matID, int32 lightID)
		: IEntity(lightID, name, transform)
		, mDisk(radius)
		, mMaterialID(matID)
		, mPDF_Cache(radius > PR_EPSILON ? 1.0f / worldSurfaceArea() : 0)
	{
	}
	virtual ~DiskEntity() {}

	std::string type() const override
	{
		return "disk";
	}

	float localSurfaceArea(uint32 id) const override
	{
		if (id == PR_INVALID_ID || id == mMaterialID)
			return mDisk.surfaceArea();
		else
			return 0;
	}

	bool isCollidable() const override
	{
		return mDisk.isValid();
	}

	float collisionCost() const override
	{
		return 1;
	}

	BoundingBox localBoundingBox() const override
	{
		return mDisk.toBoundingBox();
	}

	GeometryRepr constructGeometryRepresentation(const GeometryDev& dev) const override
	{
		const Vector3f center = transform() * Vector3f(0, 0, 0);
		const Vector3f norm	  = normalMatrix() * mDisk.normal();

		auto geom = rtcNewGeometry(dev, RTC_GEOMETRY_TYPE_ORIENTED_DISC_POINT);

		float* vertices = (float*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT4, sizeof(float) * 4, 1);
		vertices[0]		= center.x();
		vertices[1]		= center.y();
		vertices[2]		= center.z();
		vertices[3]		= mDisk.radius(); // TODO: Not affected by the transform?

		float* normal = (float*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_NORMAL, 0, RTC_FORMAT_FLOAT3, sizeof(float) * 3, 1);
		normal[0]	  = norm.x();
		normal[1]	  = norm.y();
		normal[2]	  = norm.z();

		rtcCommitGeometry(geom);
		return GeometryRepr(geom);
	}

	EntitySamplePoint sampleParameterPoint(const Vector2f& rnd) const override
	{
		return EntitySamplePoint(transform() * mDisk.surfacePoint(rnd(0), rnd(1)),
								 rnd, 0, mPDF_Cache);
	}

	float sampleParameterPointPDF() const override { return mPDF_Cache; }

	void provideGeometryPoint(const EntityGeometryQueryPoint& query,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		pt.N = (normalMatrix() * mDisk.normal()).normalized();
		Tangent::frame(pt.N, pt.Nx, pt.Ny);

		pt.UV		   = query.UV;
		pt.PrimitiveID = 0;
		pt.MaterialID  = mMaterialID;
		pt.EmissionID  = emissionID();
		pt.DisplaceID  = 0;
	}

private:
	const Disk mDisk;
	const uint32 mMaterialID;
	const float mPDF_Cache;
};

class DiskEntityPlugin : public IEntityPlugin {
public:
	std::shared_ptr<IEntity> create(const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.parameters();

		std::string name = params.getString("name", "__unnamed__");
		float radius	 = params.getNumber("radius", 1);

		const uint32 matID = ctx.lookupMaterialID(params.getParameter("material"));
		const uint32 emsID = ctx.lookupEmissionID(params.getParameter("emission"));

		return std::make_shared<DiskEntity>(name, ctx.transform(), radius, matID, emsID);
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "disk" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::DiskEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)