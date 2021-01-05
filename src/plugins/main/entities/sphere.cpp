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

	SphereEntity(const std::string& name, const Transformf& transform,
				 float r,
				 uint32 matID, uint32 lightID)
		: IEntity(lightID, name, transform)
		, mSphere(r)
		, mMaterialID(matID)
		, mOptimizeSampling(true)
		, mPDF_Cache(r > PR_EPSILON ? 1 / worldSurfaceArea(PR_INVALID_ID) : 0.0f)
	{
	}

	virtual ~SphereEntity() {}

	std::string type() const override
	{
		return "sphere";
	}

	virtual float localSurfaceArea(uint32 id) const override
	{
		if (id == PR_INVALID_ID || id == mMaterialID)
			return mSphere.surfaceArea();
		else
			return 0;
	}

	virtual float worldSurfaceArea(uint32 id) const override
	{
		if (id == PR_INVALID_ID || id == mMaterialID) {
			Eigen::Matrix3f sca;
			transform().computeRotationScaling((Eigen::Matrix3f*)nullptr, &sca);

			const Vector3f s = sca.diagonal();
			const float a	 = s(0) * mSphere.radius();
			const float b	 = s(1) * mSphere.radius();
			const float c	 = s(2) * mSphere.radius();

			// Knud Thomsen’s Formula
			const float t = (std::pow(a * b, P) + std::pow(a * c, P) + std::pow(b * c, P)) / 3;
			return 4 * PR_PI * std::pow(t, 1 / P);
		} else {
			return 0;
		}
	}

	bool isCollidable() const override
	{
		return mSphere.radius() >= PR_EPSILON;
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

		// TODO: Allow ellipsoid?
		const float scale = (transform().linear().col(0).norm()
							 + transform().linear().col(1).norm()
							 + transform().linear().col(2).norm())
							/ 3.0f;

		float* vertices = (float*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT4, sizeof(float) * 4, 1);
		vertices[0]		= center.x();
		vertices[1]		= center.y();
		vertices[2]		= center.z();
		vertices[3]		= mSphere.radius() * scale;

		rtcCommitGeometry(geom);
		return GeometryRepr(geom);
	}

	EntitySamplePoint sampleParameterPoint(const EntitySamplingInfo& info, const Vector2f& rnd) const override
	{
		Vector3f n		 = mSphere.normalPoint(rnd(0), rnd(1));
		Vector3f local_o = (invTransform() * info.Origin).normalized();

		if (local_o.dot(n) < -PR_EPSILON)
			n = -n;

		return EntitySamplePoint(transform() * (mSphere.radius() * n),
								 Spherical::uv_from_normal(n), 0, 2 * mPDF_Cache);
	}

	float sampleParameterPointPDF(const Vector3f&, const EntitySamplingInfo&) const override { return 2 * mPDF_Cache; }

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

		Tangent::frame(pt.N, pt.Nx, pt.Ny);

		const Vector2f uv = Spherical::uv_from_normal(pt.N);
		pt.UV			  = uv;
		pt.PrimitiveID	  = 0;
		pt.MaterialID	  = mMaterialID;
		pt.EmissionID	  = emissionID();
		pt.DisplaceID	  = 0;
	}

	inline void optimizeSampling(bool b) { mOptimizeSampling = b; }

private:
	const Sphere mSphere;
	const uint32 mMaterialID;
	bool mOptimizeSampling;

	const float mPDF_Cache;
};

class SphereEntityPlugin : public IEntityPlugin {
public:
	std::shared_ptr<IEntity> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();
		std::string name			 = params.getString("name", "__unnamed__");
		float r						 = params.getNumber("radius", 1.0f);

		const uint32 matID = ctx.lookupMaterialID(params.getParameter("material"));
		const uint32 emsID = ctx.lookupEmissionID(params.getParameter("emission"));

		auto obj = std::make_shared<SphereEntity>(name, ctx.transform(), r, matID, emsID);
		obj->optimizeSampling(params.getBool("optimize_sampling", true));
		return obj;
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "sphere" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Sphere Entity", "A solid sphere")
			.Identifiers(getNames())
			.Inputs()
			.Number("radius", "Radius of the sphere", 1)
			.MaterialReference("material", "Material")
			.EmissionReference("emission", "Emission", true)
			.Bool("optimize_sampling", "Should view dependent sampling be used?", true)
			.Specification()
			.get();
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SphereEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)