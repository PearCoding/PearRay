#include "geometry/Sphere.h"
#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "emission/IEmission.h"
#include "entity/GeometryDev.h"
#include "entity/GeometryRepr.h"
#include "entity/IEntity.h"
#include "entity/IEntityPlugin.h"
#include "geometry/GeometryPoint.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Spherical.h"
#include "math/Tangent.h"

namespace PR {
constexpr float P = 1.6075f;

class SphereEntity : public IEntity {
public:
	ENTITY_CLASS

	SphereEntity(uint32 id, const std::string& name, float r,
				 int32 matID, int32 lightID)
		: IEntity(id, name)
		, mSphere(r)
		, mMaterialID(matID)
		, mLightID(lightID)
		, mOptimizeSampling(true)
		, mPDF_Cache(0.0f)
	{
	}
	virtual ~SphereEntity() {}

	std::string type() const override
	{
		return "sphere";
	}

	bool isLight() const override
	{
		return mLightID >= 0;
	}

	float surfaceArea(uint32 id) const override
	{
		if (id == 0 || mMaterialID < 0 || id == (uint32)mMaterialID) {
			Eigen::Matrix3f sca;
			transform().computeRotationScaling((Eigen::Matrix3f*)nullptr, &sca);

			const auto s = flags() & EF_LocalArea ? Vector3f(1, 1, 1) : sca.diagonal();

			const float a = s(0) * mSphere.radius();
			const float b = s(1) * mSphere.radius();
			const float c = s(2) * mSphere.radius();

			// Knud Thomsen’s Formula
			const float t = (std::pow(a * b, P) + std::pow(a * c, P) + std::pow(b * c, P)) / 3;
			return 4 * PR_PI * std::pow(t, 1 / P);
		} else {
			return 0;
		}
	}

	bool isCollidable() const override
	{
		return mMaterialID > 0 && mSphere.radius() >= PR_EPSILON;
	}

	float collisionCost() const override
	{
		return 1;
	}

	BoundingBox localBoundingBox() const override
	{
		return BoundingBox(Vector3f(mSphere.radius(), mSphere.radius(), mSphere.radius()),
						   Vector3f(-mSphere.radius(), -mSphere.radius(), -mSphere.radius()));
	}

	GeometryRepr constructGeometryRepresentation(const GeometryDev& dev) const override
	{
		const Vector3f center = transform() * Vector3f(0, 0, 0);

		auto geom = rtcNewGeometry(dev, RTC_GEOMETRY_TYPE_SPHERE_POINT);

		float* vertices = (float*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT4, sizeof(float) * 4, 1);
		vertices[0]		= center.x();
		vertices[1]		= center.y();
		vertices[2]		= center.z();
		vertices[3]		= mSphere.radius(); // TODO: Not affected by the transform?

		rtcCommitGeometry(geom);
		return GeometryRepr(geom);
	}

	EntitySamplePoint sampleParameterPoint(const EntitySamplingInfo& info, const Vector2f& rnd) const override
	{
		Vector3f n = mSphere.normalPoint(rnd(0), rnd(1));
		Vector3f local_o = (invTransform() * info.Origin).normalized();

		if (local_o.dot(n) < -PR_EPSILON)
			n = -n;

		return EntitySamplePoint(transform() * (mSphere.radius()*n),
								 Spherical::uv_from_normal(n), 0, 2*mPDF_Cache);
	}

	float sampleParameterPointPDF(const EntitySamplingInfo&) const override { return 2*mPDF_Cache; }

	EntitySamplePoint sampleParameterPoint(const Vector2f& rnd) const override
	{
		return EntitySamplePoint(transform() * mSphere.surfacePoint(rnd(0), rnd(1)),
								 rnd, 0, mPDF_Cache);
	}

	float sampleParameterPointPDF() const override { return mPDF_Cache; }

	void provideGeometryPoint(const EntityGeometryQueryPoint& query,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		pt.N = (query.Position - transform() * Vector3f(0, 0, 0)).normalized();
		pt.N.normalize();

		Tangent::frame(pt.N, pt.Nx, pt.Ny);
		const Vector2f uv = Spherical::uv_from_normal(pt.N);
		pt.UV			  = uv;
		pt.EntityID		  = id();
		pt.PrimitiveID	  = query.PrimitiveID;
		pt.MaterialID	  = mMaterialID;
		pt.EmissionID	  = mLightID;
		pt.DisplaceID	  = 0;
	}

	inline void optimizeSampling(bool b) { mOptimizeSampling = b; }
	void beforeSceneBuild() override
	{
		IEntity::beforeSceneBuild();

		const float area = surfaceArea(0);

		mPDF_Cache = area > PR_EPSILON ? 1.0f / area : 0;
	}

private:
	Sphere mSphere;
	int32 mMaterialID;
	int32 mLightID;
	bool mOptimizeSampling;

	float mPDF_Cache;
};

class SphereEntityPlugin : public IEntityPlugin {
public:
	std::shared_ptr<IEntity> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;
		std::string name			 = params.getString("name", "__unnamed__");
		float r						 = params.getNumber("radius", 1.0f);

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

		auto obj = std::make_shared<SphereEntity>(id, name, r, matID, emsID);
		obj->optimizeSampling(params.getBool("optimize_sampling", true));
		return obj;
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "sphere" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SphereEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)