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
#include "math/ImportanceSampling.h"
#include "math/Projection.h"
#include "math/Tangent.h"

namespace PR {

#define PR_USE_SPHERICAL_RECTANGLE_SAMPLING
class PlaneEntity : public IEntity {
public:
	ENTITY_CLASS

	PlaneEntity(const std::string& name, const Transformf& transform,
				const Vector3f& xAxis, const Vector3f& yAxis,
				uint32 matID, uint32 lightID)
		: IEntity(lightID, name, transform)
		, mPlane(Vector3f::Zero(), xAxis, yAxis)
		, mMaterialID(matID)
		, mPDF_Cache(0.0f)
	{
		cache();
	}

	virtual ~PlaneEntity() {}

	std::string type() const override
	{
		return "plane";
	}

	virtual float localSurfaceArea(uint32 id) const override
	{
		if (id == PR_INVALID_ID || id == mMaterialID)
			return mPlane.surfaceArea();
		else
			return 0;
	}

	virtual float worldSurfaceArea(uint32 id) const override
	{
		if (id == PR_INVALID_ID || id == mMaterialID)
			return (transform().linear() * mPlane.xAxis()).norm() * (transform().linear() * mPlane.yAxis()).norm();
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

	virtual BoundingBox localBoundingBox() const override
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
		float b0, b1, k;
		float S;
	};
	inline static float safe_acos(float a) { return std::acos(std::max(-1.0f, std::min(1.0f, a))); }

	inline SphericalQuad computeSQ(const Vector3f& o) const
	{
		SphericalQuad sq;
		sq.o			 = o;
		sq.n			 = mEz;
		const Vector3f d = mS - sq.o;
		sq.x0			 = d.dot(mEx);
		sq.y0			 = d.dot(mEy);
		sq.z0			 = d.dot(sq.n);
		sq.x1			 = sq.x0 + mWidth;
		sq.y1			 = sq.y0 + mHeight;

		if (sq.z0 > 0.0f) {
			sq.z0 = -sq.z0;
			sq.n  = -sq.n;
		}

		// Optimization used in Blender Cycles
		const Eigen::Array4f diff = Eigen::Array4f(sq.x0, sq.y1, sq.x1, sq.y0) - Eigen::Array4f(sq.x1, sq.y0, sq.x0, sq.y1);
		Eigen::Array4f nz		  = Eigen::Array4f(sq.y0, sq.x1, sq.y1, sq.x0) * diff;
		nz /= sqrt(sq.z0 * sq.z0 * diff * diff + nz * nz);

		// compute internal angles (gamma_i)
		const float g0 = safe_acos(-nz.x() * nz.y());
		const float g1 = safe_acos(-nz.y() * nz.z());
		const float g2 = safe_acos(-nz.z() * nz.w());
		const float g3 = safe_acos(-nz.w() * nz.x());

		// compute predefined constants
		sq.b0 = nz.x();
		sq.b1 = nz.z();
		sq.k  = 2 * PR_PI - g2 - g3;
		sq.S  = g0 + g1 - sq.k;

		return sq;
	}

	EntitySamplePoint sampleParameterPoint(const EntitySamplingInfo& info, const Vector2f& rnd) const override
	{
		const SphericalQuad sq = computeSQ(info.Origin);

		// 1. compute ’cu’
		const float au = std::fma(rnd(0), sq.S, sq.k);
		const float fu = std::fma(std::cos(au), sq.b0, -sq.b1) / std::sin(au);
		const float cu = std::min(1.0f, std::max(-1.0f,
												 std::copysign(1.0f, fu) / std::sqrt(sumProd(fu, fu, sq.b0, sq.b0))));

		// 2. compute ’xu’
		const float xu = std::min(sq.x1, std::max(sq.x0,
												  -(cu * sq.z0) / std::max(1e-7f, std::sqrt(std::fma(-cu, cu, 1.0f)))));

		// 3. compute ’yv’
		const float d		= std::sqrt(sumProd(xu, xu, sq.z0, sq.z0));
		const float h0		= sq.y0 / std::sqrt(sumProd(d, d, sq.y0, sq.y0));
		const float h1		= sq.y1 / std::sqrt(sumProd(d, d, sq.y1, sq.y1));
		const float hv		= std::fma(rnd(1), h1 - h0, h0);
		const float hv2		= hv * hv;
		constexpr float eps = 1e-6f;
		const float yv		= (hv2 < 1.0f - eps) ? (hv * d) / std::sqrt(1.0f - hv2) : sq.y1;

		// 4. transform (xu,yv,z0) to entity local coords
		const Vector3f p = sq.o + xu * mEx + yv * mEy + sq.z0 * sq.n;

		// 5. Transform pdf_s to pdf_a
		const float pdf_s = sq.S > PR_EPSILON ? 1 / sq.S : 0.0f;
		const Vector3f L  = (p - info.Origin);
		const float dist2 = L.squaredNorm();
		const float ndotv = L.normalized().dot(normalMatrix() * mPlane.normal());
		const float pdf_a = IS::toArea(pdf_s, dist2, std::abs(ndotv));

		PR_ASSERT(pdf_a >= 0.0, "PDF has to be positive");
		return EntitySamplePoint(p, mPlane.project(invTransform() * p), 0, pdf_a);
	}

	float sampleParameterPointPDF(const Vector3f& p, const EntitySamplingInfo& info) const override
	{
		const float s	  = computeSQ(info.Origin).S;
		const float pdf_s = s > PR_EPSILON ? 1 / s : 0.0f;
		const Vector3f L  = (p - info.Origin);
		const float dist2 = L.squaredNorm();
		const float ndotv = L.normalized().dot(normalMatrix() * mPlane.normal());
		const float pdf_a = IS::toArea(pdf_s, dist2, std::abs(ndotv));

		PR_ASSERT(pdf_a >= 0.0, "PDF has to be positive");
		return pdf_a;
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

		pt.N  = mEz;
		pt.Nx = mEx;
		pt.Ny = mEy;

		pt.UV		   = query.UV;
		pt.PrimitiveID = 0;
		pt.MaterialID  = mMaterialID;
		pt.EmissionID  = emissionID();
		pt.DisplaceID  = 0;
	}

	inline void centerOn()
	{
		mPlane.setPosition(-0.5f * mPlane.xAxis() - 0.5f * mPlane.yAxis());
		cache();
	}

	void cache()
	{
		mS		= transform() * mPlane.position();
		mEx		= transform().linear() * mPlane.xAxis();
		mEy		= transform().linear() * mPlane.yAxis();
		mEz		= normalMatrix() * mPlane.normal();
		mWidth	= mEx.norm();
		mHeight = mEy.norm();

		mEx.normalize();
		mEy.normalize();
		mEz.normalize();

		const float garea = worldSurfaceArea(PR_INVALID_ID);
		mPDF_Cache		  = (garea > PR_EPSILON ? 1.0f / garea : 0);
	}

private:
	Plane mPlane;

	Vector3f mS;
	Vector3f mEx;
	Vector3f mEy;
	Vector3f mEz;
	float mWidth;
	float mHeight;

	const uint32 mMaterialID;

	float mPDF_Cache;
}; // namespace PR

class PlaneEntityPlugin : public IEntityPlugin {
public:
	std::shared_ptr<IEntity> create(const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.parameters();

		std::string name = params.getString("name", "__unnamed__");
		Vector3f xAxis	 = params.getVector3f("x_axis", Vector3f(1, 0, 0));
		Vector3f yAxis	 = params.getVector3f("y_axis", Vector3f(0, 1, 0));
		float width		 = params.getNumber("width", 1);
		float height	 = params.getNumber("height", 1);

		const uint32 matID = ctx.lookupMaterialID(params.getParameter("material"));
		const uint32 emsID = ctx.lookupEmissionID(params.getParameter("emission"));

		auto obj = std::make_shared<PlaneEntity>(name, ctx.transform(), width * xAxis, height * yAxis, matID, emsID);
		if (params.getBool("centering", false))
			obj->centerOn();

		return obj;
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "plane" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Plane Entity", "A solid plane")
			.Identifiers(getNames())
			.Inputs()
			.Number("width", "Local width", 1)
			.Number("height", "Local height", 1)
			.VectorV({ "x_axis", "axis_x" }, "Local x axis", Vector3f(1, 0, 0))
			.VectorV({ "y_axis", "axis_y" }, "Local y axis", Vector3f(0, 1, 0))
			.MaterialReference("material", "Material")
			.EmissionReference("emission", "Emission", true)
			.Bool("centering", "Set origin of the plane at the center of the constructed plane. Useful for area lights", false)
			.Specification()
			.get();
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::PlaneEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)