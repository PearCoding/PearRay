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

#define PR_USE_SPHERICAL_RECTANGLE_SAMPLING
class PlaneEntity : public IEntity {
public:
	ENTITY_CLASS

	PlaneEntity(uint32 id, const std::string& name,
				const Vector3f& xAxis, const Vector3f& yAxis,
				int32 matID, int32 lightID)
		: IEntity(id, name)
		, mPlane(Vector3f(0, 0, 0), xAxis, yAxis)
		, mEx(xAxis.normalized())
		, mEy(yAxis.normalized())
		, mWidth(xAxis.norm())
		, mHeight(yAxis.norm())
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

// Ureña, C., Fajardo, M. and King, A. (2013),
// An Area‐Preserving Parametrization for Spherical Rectangles.
// Computer Graphics Forum, 32: 59-66. doi:10.1111/cgf.12151
#ifdef PR_USE_SPHERICAL_RECTANGLE_SAMPLING
	struct SphericalQuad {
		Vector3f o, n;
		float z0;
		float x0, y0;
		float x1, y1;
		double b0, b1, k;
		double S;
	};
	inline SphericalQuad computeSQ(const Vector3f& o) const
	{
		SphericalQuad sq;
		sq.o			 = invTransform() * o;
		const Vector3f d = mPlane.position() - sq.o;
		sq.x0			 = d.dot(mEx);
		sq.y0			 = d.dot(mEy);
		sq.z0			 = d.dot(mPlane.normal());
		sq.x1			 = sq.x0 + mWidth;
		sq.y1			 = sq.y0 + mHeight;

		sq.n = mPlane.normal();
		if (sq.z0 > 0.0f) {
			sq.z0 = -sq.z0;
			sq.n  = -sq.n;
		}

		// create vectors to four vertices
		const Vector3f v[2][2] = {
			{ Vector3f(sq.x0, sq.y0, sq.z0), Vector3f(sq.x0, sq.y1, sq.z0) },
			{ Vector3f(sq.x1, sq.y0, sq.z0), Vector3f(sq.x1, sq.y1, sq.z0) }
		};

		// compute normals to edges
		const Vector3f k[4] = {
			v[0][0].cross(v[1][0]).normalized(),
			v[1][0].cross(v[1][1]).normalized(),
			v[1][1].cross(v[0][1]).normalized(),
			v[0][1].cross(v[0][0]).normalized()
		};

		// compute internal angles (gamma_i)
		const float g[4] = {
			std::acos(-k[0].dot(k[1])),
			std::acos(-k[1].dot(k[2])),
			std::acos(-k[2].dot(k[3])),
			std::acos(-k[3].dot(k[0]))
		};

		// compute predefined constants
		sq.b0 = k[0](2);
		sq.b1 = k[2](2);
		sq.k  = 2 * PR_PI - g[2] - g[3];
		sq.S  = g[0] + g[1] - sq.k;

		return sq;
	}

	EntitySamplePoint sampleParameterPoint(const EntitySamplingInfo& info, const Vector2f& rnd) const override
	{
		const auto sq = computeSQ(info.Origin);
		auto safeSqrt = [](double x) {
			return std::sqrt(std::max(x, 0.0));
		};

		// 1. compute ’cu’
		const double au = rnd(0) * sq.S + sq.k;
		const double fu = (std::cos(au) * sq.b0 - sq.b1) / std::sin(au);
		const double cu = std::copysign(1.0, fu) / safeSqrt(fu * fu + sq.b0 * sq.b0);

		// 2. compute ’xu’
		const double xu = -(cu * sq.z0) / safeSqrt(1.0 - cu * cu);

		// 3. compute ’yv’
		const double d  = std::sqrt(xu * xu + sq.z0 * sq.z0);
		const double h0 = sq.y0 / std::sqrt(d * d + sq.y0 * sq.y0);
		const double h1 = sq.y1 / std::sqrt(d * d + sq.y1 * sq.y1);
		const double hv = h0 + rnd(1) * (h1 - h0);
		const double yv = hv * d / safeSqrt(1.0 - hv * hv);

		// 4. transform (xu,yv,z0) to entity local coords
		const Vector3f p = sq.o + xu * mEx + yv * mEy + sq.z0 * sq.n;

		return EntitySamplePoint(transform() * p, mPlane.project(p), 0, 1.0 / sq.S);
	}

	float sampleParameterPointPDF(const EntitySamplingInfo& info) const override
	{
		return 1.0 / computeSQ(info.Origin).S;
	}
#endif //PR_USE_SPHERICAL_RECTANGLE_SAMPLING

	EntitySamplePoint sampleParameterPoint(const Vector2f& rnd) const override
	{
		return EntitySamplePoint(transform() * mPlane.surfacePoint(rnd(0), rnd(1)),
								 rnd, 0, mPDF_Cache);
	}

	float sampleParameterPointPDF() const override { return mPDF_Cache; }

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

	const Vector3f mEx;
	const Vector3f mEy;
	const float mWidth;
	const float mHeight;

	const int32 mMaterialID;
	const int32 mLightID;

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