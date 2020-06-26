#include "geometry/Plane.h"
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

class PlaneEntity : public IEntity {
public:
	ENTITY_CLASS

	PlaneEntity(uint32 id, const std::string& name,
				const Vector3f& xAxis, const Vector3f& yAxis,
				int32 matID, int32 lightID)
		: IEntity(id, name)
		, mPlane(Vector3f(0, 0, 0), xAxis, yAxis)
		, mMaterialID(matID)
		, mLightID(lightID)
		, mPDF_Cache(0.0f)
	{
	}
	virtual ~PlaneEntity() {}

	std::string type() const override
	{
		return "plane";
	}

	bool isLight() const override
	{
		return mLightID >= 0;
	}

	float surfaceArea(uint32 id) const override
	{
		if (id == 0 || mMaterialID < 0 || id == (uint32)mMaterialID)
			return mPlane.surfaceArea();
		else
			return 0;
	}

	bool isCollidable() const override
	{
		return mPlane.isValid();
	}

	float collisionCost() const override
	{
		return 1;
	}

	BoundingBox localBoundingBox() const override
	{
		return mPlane.toBoundingBox();
	}

	GeometryRepr constructGeometryRepresentation(const GeometryDev& dev) const override
	{
		auto geom	= rtcNewGeometry(dev, RTC_GEOMETRY_TYPE_QUAD);
		auto assign = [](float* v, const Vector3f& p) {
			v[0] = p[0];
			v[1] = p[1];
			v[2] = p[2];
		};

		float* vertices = (float*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(float) * 3, 4);
		assign(&vertices[0], transform() * mPlane.position());
		assign(&vertices[3], transform() * (mPlane.position() + mPlane.yAxis()));
		assign(&vertices[6], transform() * (mPlane.position() + mPlane.yAxis() + mPlane.xAxis()));
		assign(&vertices[9], transform() * (mPlane.position() + mPlane.xAxis()));

		uint32* inds = (uint32*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT4, sizeof(uint32) * 4, 1);
		inds[0]		 = 0;
		inds[1]		 = 1;
		inds[2]		 = 2;
		inds[3]		 = 3;

		rtcCommitGeometry(geom);
		return GeometryRepr(geom);
	}

	EntityRandomPoint pickRandomParameterPoint(const Vector3f&, const Vector2f& rnd) const override
	{
		return EntityRandomPoint(transform() * mPlane.surfacePoint(rnd(0), rnd(1)),
								 rnd, 0, mPDF_Cache);
	}

	void provideGeometryPoint(const EntityGeometryQueryPoint& query,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		float u = query.UV[0];
		float v = query.UV[1];

		pt.N  = normalMatrix() * mPlane.normal();
		pt.Nx = normalMatrix() * mPlane.xAxis();
		pt.Ny = normalMatrix() * mPlane.yAxis();

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.UV		   = Vector2f(u, v);
		pt.EntityID	   = id();
		pt.PrimitiveID = query.PrimitiveID;
		pt.MaterialID  = mMaterialID;
		pt.EmissionID  = mLightID;
		pt.DisplaceID  = 0;
	}

	inline void centerOn()
	{
		mPlane.setPosition(-0.5f * mPlane.xAxis() - 0.5f * mPlane.yAxis());
	}

	void beforeSceneBuild() override
	{
		IEntity::beforeSceneBuild();

		const float area = surfaceArea(0);
		mPDF_Cache		 = (area > PR_EPSILON ? 1.0f / area : 0);
	}

private:
	Plane mPlane;
	int32 mMaterialID;
	int32 mLightID;

	float mPDF_Cache;
};

class PlaneEntityPlugin : public IEntityPlugin {
public:
	std::shared_ptr<IEntity> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;

		std::string name = params.getString("name", "__unnamed__");
		Vector3f xAxis	 = params.getVector3f("x_axis", Vector3f(1, 0, 0));
		Vector3f yAxis	 = params.getVector3f("y_axis", Vector3f(0, 1, 0));
		float width		 = params.getNumber("width", 1);
		float height	 = params.getNumber("height", 1);

		std::string emsName = params.getString("emission", "");
		std::string matName = params.getString("material", "");

		int32 matID					   = -1;
		std::shared_ptr<IMaterial> mat = ctx.Env->getMaterial(matName);
		if (mat)
			matID = mat->id();

		int32 emsID					   = -1;
		std::shared_ptr<IEmission> ems = ctx.Env->getEmission(emsName);
		if (ems)
			emsID = ems->id();

		auto obj = std::make_shared<PlaneEntity>(id, name, width * xAxis, height * yAxis, matID, emsID);
		if (params.getBool("centering", false))
			obj->centerOn();

		return obj;
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "plane" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::PlaneEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)