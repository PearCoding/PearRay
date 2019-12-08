#include "geometry/Quadric.h"
#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"
#include "geometry/CollisionData.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "registry/Registry.h"

#include <array>

namespace PR {

class QuadricEntity : public IEntity {
public:
	ENTITY_CLASS

	QuadricEntity(uint32 id, const std::string& name,
				  const std::array<float, 10>& parameters,
				  const Vector3f& minB, const Vector3f& maxB,
				  int32 matID, int32 lightID)
		: IEntity(id, name)
		, mBoundingBox(minB, maxB)
		, mParameters(parameters)
		, mMaterialID(matID)
		, mLightID(lightID)
	{
	}
	virtual ~QuadricEntity() {}

	std::string type() const override
	{
		return "quadric";
	}

	bool isLight() const override
	{
		return mLightID >= 0;
	}

	float surfaceArea(uint32 /*id*/) const override
	{
		// TODO
		return 0;
	}

	bool isCollidable() const override
	{
		return mBoundingBox.isValid();
	}

	float collisionCost() const override
	{
		return 3;
	}

	BoundingBox localBoundingBox() const override
	{
		return mBoundingBox;
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local	 = in.transformAffine(invTransform().matrix(), invTransform().linear());
		auto range		  = localBoundingBox().intersectsRange(in_local);
		bfloat validRange = range.Entry < 0;
		range.Entry		  = blend(vfloat(0), range.Entry, validRange);

		vfloat t;
		bfloat valid = Quadric::intersect<vfloat>(mParameters,
												  in_local.Origin + range.Entry * in_local.Direction,
												  in_local.Direction, t);
		valid		 = b_and(b_and(valid, t <= range.Exit), range.Successful);
		t += range.Entry;

		out.Parameter   = in_local.t(t);
		out.HitDistance = blend(t, vfloat(std::numeric_limits<float>::infinity()), valid);

		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= simdpp::make_uint(id());
		out.FaceID		= simdpp::make_uint(0);
		out.MaterialID  = simdpp::make_uint(mMaterialID);
	}

	void checkCollision(const Ray& in, SingleCollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());

		out.HitDistance = std::numeric_limits<float>::infinity();
		auto range		= localBoundingBox().intersectsRange(in_local);
		if (range.Entry < 0)
			range.Entry = 0;

		float t;
		if (range.Successful
			&& Quadric::intersect<float>(mParameters,
										 in_local.Origin + range.Entry * in_local.Direction,
										 in_local.Direction, t)) {
			t += range.Entry;
			if (t <= range.Exit) {
				out.Parameter   = in_local.t(t);
				out.HitDistance = t;
			}
		}

		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= id();
		out.FaceID		= 0;
		out.MaterialID  = mMaterialID;
	}

	Vector3f pickRandomParameterPoint(const Vector3f&, const Vector2f& rnd,
									  uint32& faceID, float& pdf) const override
	{
		// TODO
		pdf	= 1.0f;
		faceID = 0;
		return Vector3f(rnd(0), rnd(1), 0);
	}

	void provideGeometryPoint(const Vector3f&, uint32, const Vector3f& parameter,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		pt.P = transform() * parameter;
		pt.N = normalMatrix() * Quadric::normal(mParameters, parameter);

		Tangent::frame(pt.N, pt.Nx, pt.Ny);

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.UVW = parameter.cwiseAbs();
		if (pt.UVW(2) < PR_EPSILON) {
			pt.UVW(0) /= pt.UVW(2);
			pt.UVW(1) /= pt.UVW(2);
			pt.UVW(2) = 0;
		}
		pt.MaterialID = mMaterialID;
		pt.EmissionID = mLightID;
		pt.DisplaceID = 0;
	}

private:
	BoundingBox mBoundingBox;
	std::array<float, 10> mParameters;

	int32 mMaterialID;
	int32 mLightID;
};

class QuadricEntityFactory : public IEntityFactory {
public:
	std::shared_ptr<IEntity> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const auto& reg = env.registry();

		std::string name		  = reg.getForObject<std::string>(RG_ENTITY, uuid, "name",
														  "__unnamed__");
		std::vector<float> params = reg.getForObject<std::vector<float>>(RG_ENTITY, uuid, "parameters", std::vector<float>());
		Vector3f minB			  = reg.getForObject<Vector3f>(RG_ENTITY, uuid, "min",
												   Vector3f(-1, -1, -1));
		Vector3f maxB			  = reg.getForObject<Vector3f>(RG_ENTITY, uuid, "max",
												   Vector3f(1, 1, 1));

		std::string emsName = reg.getForObject<std::string>(RG_ENTITY, uuid, "emission", "");
		std::string matName = reg.getForObject<std::string>(RG_ENTITY, uuid, "material", "");

		int32 matID					   = -1;
		std::shared_ptr<IMaterial> mat = env.getMaterial(matName);
		if (mat)
			matID = mat->id();

		int32 emsID					   = -1;
		std::shared_ptr<IEmission> ems = env.getEmission(emsName);
		if (ems)
			emsID = ems->id();

		std::array<float, 10> parameters;
		if (params.size() == 3)
			parameters = { params[0], params[1], params[2], 0, 0, 0, 0, 0, 0, 0 };
		else if (params.size() == 4)
			parameters = { params[0], params[1], params[2], 0, 0, 0, 0, 0, 0, params[3] };
		else if (params.size() == 10)
			parameters = { params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7], params[8], params[9] };
		else {
			PR_LOG(L_ERROR) << "Invalid quadric parameters given" << std::endl;
			return nullptr;
		}

		return std::make_shared<QuadricEntity>(id, name, parameters, minB, maxB, matID, emsID);
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "quadric" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::QuadricEntityFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)