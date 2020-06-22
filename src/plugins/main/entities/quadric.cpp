#include "geometry/Quadric.h"
#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityPlugin.h"
#include "geometry/CollisionData.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Tangent.h"

#include <array>

// TODO: Change to new geometry representation

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

		auto in_local		= in.transformAffine(invTransform().matrix(), invTransform().linear());
		auto range			= localBoundingBox().intersectsRange(in_local);
		bfloat invalidRange = range.Entry < 0;
		range.Entry			= blend(vfloat(0), range.Entry, invalidRange);

		vfloat t;
		bfloat valid = Quadric::intersect<vfloat>(mParameters,
												  in_local.t(range.Entry),
												  in_local.Direction, out.HitDistance);
		out.HitDistance += range.Entry;
		out.Successful = b_and(b_and(valid, out.HitDistance <= range.Exit), range.Successful);

		out.Parameter = in_local.t(out.HitDistance);

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

		out.resetSuccessful();
		auto range = localBoundingBox().intersectsRange(in_local);
		if (range.Entry < 0)
			range.Entry = 0;

		float t;
		if (range.Successful
			&& Quadric::intersect<float>(mParameters,
										 in_local.t(range.Entry),
										 in_local.Direction, t)) {
			t += range.Entry;
			if (t <= range.Exit) {
				out.Parameter	= in_local.t(t);
				out.HitDistance = t;
				out.makeSuccessful();
			}
		}

		if (out.Successful) {
			out.HitDistance = in_local.transformDistance(out.HitDistance,
														 transform().linear());
			out.EntityID	= id();
			out.FaceID		= 0;
			out.MaterialID	= mMaterialID;
		}
	}

	Vector3f pickRandomParameterPoint(const Vector3f&, const Vector2f& rnd,
									  uint32& faceID, float& pdf) const override
	{
		// TODO
		pdf	   = 1.0f;
		faceID = 0;
		return Vector3f(rnd(0), rnd(1), 0);
	}

	void provideGeometryPoint(const Vector3f&, uint32, const Vector3f& parameter,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		pt.P = transform() * parameter;
		pt.N = normalMatrix() * Quadric::normal(mParameters, parameter);
		pt.N.normalize();

		Tangent::frame(pt.N, pt.Nx, pt.Ny);

		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.UVW = parameter.cwiseAbs();
		if (pt.UVW(2) < PR_EPSILON) {
			pt.UVW(0) /= pt.UVW(2);
			pt.UVW(1) /= pt.UVW(2);
			pt.UVW(2) = 0;
		}
		pt.EntityID	   = id();
		pt.PrimitiveID = query.PrimitiveID;
		pt.MaterialID  = mMaterialID;
		pt.EmissionID  = mLightID;
		pt.DisplaceID  = 0;
	}

private:
	BoundingBox mBoundingBox;
	std::array<float, 10> mParameters;

	int32 mMaterialID;
	int32 mLightID;
};

class QuadricEntityPlugin : public IEntityPlugin {
public:
	std::shared_ptr<IEntity> create(uint32 id, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;

		std::string name	  = params.getString("name", "__unnamed__");
		std::vector<float> qp = params.getNumberArray("parameters");
		Vector3f minB		  = params.getVector3f("min", Vector3f(-1, -1, -1));
		Vector3f maxB		  = params.getVector3f("max", Vector3f(1, 1, 1));

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

PR_PLUGIN_INIT(PR::QuadricEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)