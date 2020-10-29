#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityPlugin.h"
#include "geometry/CollisionData.h"
#include "geometry/Disk.h"
#include "geometry/Quadric.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Tangent.h"

#include "sampler/SplitSample.h"

// TODO: Change to new geometry representation

namespace PR {

class ConeEntity : public IEntity {
public:
	ENTITY_CLASS

	ConeEntity(uint32 id, const std::string& name,
			   float radius, float height,
			   uint32 matID, uint32 lightID)
		: IEntity(id, lightID, name)
		, mDisk(radius)
		, mHeight(height)
		, mMaterialID(matID)
		, mOffset(0, 0, 0)
		, mPDF_Cache(0.0f)
	{
	}
	virtual ~ConeEntity() {}

	std::string type() const override
	{
		return "cone";
	}

	// TODO: Local surface -> Global surface
	float localSurfaceArea(uint32 id) const override
	{
		constexpr float K = 150.0f / 360;
		const float L2	  = mHeight * mHeight + mDisk.radius() * mDisk.radius();

		if (id == PR_INVALID_ID || id == mMaterialID)
			return mDisk.localSurfaceArea() + K * PR_PI * L2;
		else
			return 0;
	}

	bool isCollidable() const override
	{
		return mDisk.isValid() && mHeight > PR_EPSILON;
	}

	float collisionCost() const override
	{
		return 2;
	}

	BoundingBox localBoundingBox() const override
	{
		BoundingBox box = mDisk.toBoundingBox();
		box.combine(Vector3f(0, 0, mHeight));
		return box;
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());
		in_local.Origin += promote(mOffset);

		mDisk.intersects(in_local, out);

		auto range		  = localBoundingBox().intersectsRange(in_local);
		bfloat validRange = range.Entry < 0;
		range.Entry		  = blend(vfloat(0), range.Entry, validRange);

		vfloat qu_t;
		bfloat valid = Quadric::intersect<vfloat>(
			constructConeParameters(),
			in_local.Origin + range.Entry * in_local.Direction,
			in_local.Direction,
			qu_t);
		qu_t += range.Entry;
		valid = b_and(b_and(b_and(valid, qu_t <= range.Exit), range.Successful), qu_t < out.HitDistance);

		out.FaceID		= blend(vuint32(1), vuint32(0), (buint32)valid);
		out.HitDistance = blend(qu_t, out.HitDistance, valid);
		out.Parameter	= in_local.t(out.HitDistance);
		out.Successful	= out.Successful | valid;

		out.HitDistance = in_local.transformDistance(out.HitDistance,
													 transform().linear());
		out.EntityID	= simdpp::make_uint(id());
		out.FaceID		= simdpp::make_uint(0);
		out.MaterialID	= simdpp::make_uint(mMaterialID);
	}

	void checkCollision(const Ray& in, HitPoint& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());
		in_local.Origin += mOffset;

		mDisk.intersects(in_local, out);

		auto range = localBoundingBox().intersectsRange(in_local);
		if (range.Entry < 0)
			range.Entry = 0;

		float qu_t;
		if (range.Successful
			&& Quadric::intersect<float>(
				constructConeParameters(),
				in_local.Origin + range.Entry * in_local.Direction,
				in_local.Direction,
				qu_t)) {

			qu_t += range.Entry;
			if ((!out.Successful || qu_t < out.HitDistance) && qu_t <= range.Exit) {
				out.FaceID		= 1;
				out.HitDistance = qu_t;
				out.Parameter	= in_local.t(out.HitDistance);
			}
		} else {
			out.FaceID = 0;
		}

		out.HitDistance = in_local.transformDistance(out.HitDistance,
													 transform().linear());
		out.EntityID	= id();
		out.MaterialID	= mMaterialID;
	}

	Vector3f pickRandomParameterPoint(const Vector3f&, const Vector2f& rnd,
									  uint32& faceID, float& pdf) const override
	{
		SplitSample2D split(rnd, 0, 2);

		pdf	   = mPDF_Cache;
		faceID = split.integral1();
		// TODO Specific sampling for disk and cone

		return Vector3f(split.uniform1(), split.uniform2(), 0);
	}

	void provideGeometryPoint(const Vector3f&, uint32 face, const Vector3f& parameter,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		float u = parameter[0];
		float v = parameter[1];

		if (face == 0) { // Disk
			pt.P   = transform() * (mDisk.surfacePoint(u, v) - mOffset);
			pt.N   = normalMatrix() * (-mDisk.normal());
			pt.UVW = Vector3f(u, v, 0);
		} else {
			pt.P = transform() * (parameter - mOffset);
			pt.N = normalMatrix() * Quadric::normal(constructConeParameters(), parameter);
			// TODO: Better uv
			pt.UVW = parameter.cwiseAbs();
			if (pt.UVW(2) < PR_EPSILON) {
				pt.UVW(0) /= pt.UVW(2);
				pt.UVW(1) /= pt.UVW(2);
				pt.UVW(2) = 0;
			}
		}

		pt.N.normalize();
		Tangent::frame(pt.N, pt.Nx, pt.Ny);

		pt.EntityID	   = id();
		pt.PrimitiveID = query.PrimitiveID;
		pt.MaterialID  = mMaterialID;
		pt.EmissionID  = emissionID();
		pt.DisplaceID  = 0;
	}

	void beforeSceneBuild() override
	{
		IEntity::beforeSceneBuild();

		const float area = localSurfaceArea(0);
		mPDF_Cache		 = (area > PR_EPSILON ? 1.0f / area : 0);
	}

	inline void centerOn()
	{
		mOffset = Vector3f(0, 0, mHeight / 2);
	}

private:
	inline Quadric::ParameterArray constructConeParameters() const
	{
		const float a				 = 1 / (mDisk.radius() * mDisk.radius());
		const float b				 = 1 / (mHeight * mHeight);
		Quadric::ParameterArray CONE = { a, a, -b, 0, 0, 0, 0, 0, 2 / mHeight, -1 };
		return CONE;
	}

	Disk mDisk;
	float mHeight;

	uint32 mMaterialID;
	Vector3f mOffset;

	float mPDF_Cache;
};

class ConeEntityPlugin : public IEntityPlugin {
public:
	std::shared_ptr<IEntity> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.parameters();

		std::string name = params.getString("name", "__unnamed__cone__");
		float radius	 = params.getNumber("radius", 1);
		float height	 = params.getNumber("height", 1);

		std::string emsName = params.getString("emission", "");
		std::string matName = params.getString("material", "");

		uint32 matID				   = PR_INVALID_ID;
		std::shared_ptr<IMaterial> mat = ctx.Env->getMaterial(matName);
		if (mat)
			matID = mat->id();

		uint32 emsID				   = PR_INVALID_ID;
		std::shared_ptr<IEmission> ems = ctx.Env->getEmission(emsName);
		if (ems)
			emsID = ems->id();

		auto obj = std::make_shared<ConeEntity>(id, name, radius, height, matID, emsID);
		if (params.getBool("center_on", false))
			obj->centerOn();

		return obj;
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "cone" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::ConeEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)