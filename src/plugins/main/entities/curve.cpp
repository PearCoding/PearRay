#include "curve/Curve.h"
#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"
#include "geometry/CollisionData.h"
#include "material/IMaterial.h"
#include "math/Projection.h"

#include <boost/filesystem.hpp>

namespace PR {

class CurveEntity : public IEntity {
public:
	ENTITY_CLASS

	CurveEntity(uint32 id, const std::string& name,
				const Curve3& curve,
				Vector2f width,
				int32 materialID, int32 lightID)
		: IEntity(id, name)
		, mLightID(lightID)
		, mMaterialID(materialID)
		, mCurve(curve)
		, mWidth(width)
	{
	}

	virtual ~CurveEntity() {}

	std::string type() const override
	{
		return "curve";
	}

	bool isLight() const override
	{
		return mLightID >= 0;
	}

	float surfaceArea(uint32 id) const override
	{
		// TODO
		return 0;
	}

	bool isCollidable() const override
	{
		return mMaterialID >= 0 && mBoundingBox_Cache.isValid();
	}

	float collisionCost() const override
	{
		return 10;
	}

	BoundingBox localBoundingBox() const override
	{
		return mBoundingBox_Cache;
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());

		// TODO

		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.FaceID		= simdpp::make_uint(0);
		out.EntityID	= simdpp::make_uint(id());
		out.MaterialID  = simdpp::make_uint(mMaterialID);
	}

	void checkCollision(const Ray& in, SingleCollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());

		// TODO

		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.FaceID		= 0;
		out.EntityID	= id();
		out.MaterialID  = mMaterialID;
	}

	Vector2f pickRandomPoint(const Vector2f& rnd, uint32& faceID, float& pdf) const override
	{
		PR_PROFILE_THIS;
		// TODO
		return Vector2f(0, 0);
	}

	void provideGeometryPoint(uint32 /*faceID*/, float u, float v,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		// TODO

		// Global
		pt.P  = transform() * pt.P;
		pt.N  = normalMatrix() * pt.N;
		pt.Nx = normalMatrix() * pt.Nx;
		pt.Ny = normalMatrix() * pt.Ny;

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.MaterialID = mMaterialID;
		pt.EmissionID = mLightID;
		pt.DisplaceID = 0;
	}

	void beforeSceneBuild() override
	{
		IEntity::beforeSceneBuild();

		mBoundingBox_Cache = BoundingBox();
		for (size_t i = 0; i <= mCurve.degree(); ++i)
			mBoundingBox_Cache.combine(mCurve.point(i));

		mBoundingBox_Cache.expand(std::max(mWidth(0), mWidth(1)) * 0.5f);
	}

private:
	int32 mLightID;
	int32 mMaterialID;
	Curve3 mCurve;
	Vector2f mWidth;

	BoundingBox mBoundingBox_Cache;
};

class CurveEntityFactory : public IEntityFactory {
public:
	std::shared_ptr<IEntity> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const Registry& reg = env.registry();
		std::string name	= reg.getForObject<std::string>(RG_ENTITY, uuid, "name", "__unnamed__");
		uint32 degree		= (uint32)std::max(
			  1,
			  reg.getForObject<int>(RG_ENTITY, uuid, "degree", 3));

		std::vector<float> points = reg.getForObject<std::vector<float>>(
			RG_ENTITY, uuid, "points", std::vector<float>());
		if (points.size() != (degree + 1) * 3) {
			PR_LOG(L_ERROR) << "Curve " << name << " is invalid. Expected " << (degree + 1) * 3 << " entries in points" << std::endl;
			return nullptr;
		}

		Vector2f width = reg.getForObject<Vector2f>(RG_ENTITY, uuid, "width", Vector2f(1, 1));

		std::string matName = reg.getForObject<std::string>(
			RG_ENTITY, uuid, "material", "");
		int32 matID					   = -1;
		std::shared_ptr<IMaterial> mat = env.getMaterial(matName);
		if (mat)
			matID = static_cast<int32>(mat->id());

		std::string emsName = reg.getForObject<std::string>(
			RG_ENTITY, uuid, "emission", "");
		int32 emsID					   = -1;
		std::shared_ptr<IEmission> ems = env.getEmission(emsName);
		if (ems)
			emsID = static_cast<int32>(ems->id());

		Curve3 curve;
		Curve3::PointList list(points.size() / 3);
		for (size_t i = 0; i < list.size(); ++i)
			list[i] = Vector3f(points[3 * i], points[3 * i + 1], points[3 * i + 2]);
		curve.setPoints(list);

		return std::make_shared<CurveEntity>(id, name,
											 curve, width,
											 matID, emsID);
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "curve", "bezier" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::CurveEntityFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)