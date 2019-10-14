#include "Environment.h"
#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"
#include "math/Projection.h"
#include "mesh/TriMesh.h"
#include "sampler/SplitSample.h"

namespace PR {

class MeshEntity : public IEntity {
public:
	ENTITY_CLASS

	MeshEntity(uint32 id, const std::string& name,
			   const std::shared_ptr<TriMesh>& mesh)
		: IEntity(id, name)
		, mLightID(0)
		, mMesh(mesh)
	{
		PR_ASSERT(mesh->isBuilt(), "Expect mesh to be built!");
	}
	virtual ~MeshEntity() {}

	std::string type() const override
	{
		return "mesh";
	}

	bool isLight() const override
	{
		return mLightID != 0;
	}

	float surfaceArea(uint32 id) const override
	{
		return mMesh->surfaceArea(id, transform());
	}

	bool isCollidable() const override
	{
		return mMesh->isValid() && mMesh->isBuilt();
	}

	float collisionCost() const override
	{
		return mMesh->collisionCost();
	}

	BoundingBox localBoundingBox() const override
	{
		return mMesh->boundingBox();
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		auto in_local = in.transform(invTransform().matrix(), invDirectionMatrix());
		mMesh->checkCollision(in_local, out);

		// out.FaceID is set inside mesh
		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= simdpp::make_uint(id());
		out.MaterialID  = simdpp::make_uint(0); //TODO
	}

	void checkCollision(const Ray& in, SingleCollisionOutput& out) const override
	{
		auto in_local = in.transform(invTransform().matrix(), invDirectionMatrix());
		mMesh->checkCollision(in_local, out);

		// out.FaceID is set inside mesh
		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= id();
		out.MaterialID  = 0; //TODO
	}

	Vector3f pickRandomPoint(const Vector2f& rnd, float& pdf) const override
	{
		SplitSample2D split(rnd, 0, mMesh->faceCount());
		Face face = mMesh->getFace(split.integral1());

		pdf = 1.0f / (mMesh->faceCount() * face.surfaceArea());
		return transform() * face.interpolateVertices(rnd(0), rnd(1));
	}

	void provideGeometryPoint(uint32 faceID, float u, float v,
							  GeometryPoint& pt) const override
	{
		mMesh->provideGeometryPoint(faceID, u, v, pt);

		pt.P  = transform() * pt.P;
		pt.N  = directionMatrix() * pt.N;
		pt.Nx = directionMatrix() * pt.Nx;
		pt.Ny = directionMatrix() * pt.Ny;

		pt.MaterialID = 0; //TODO
	}

protected:
	void onFreeze(RenderContext* context) override
	{
		IEntity::onFreeze(context);
	}

private:
	uint32 mLightID;

	std::shared_ptr<TriMesh> mMesh;
};

class MeshEntityFactory : public IEntityFactory {
public:
	std::shared_ptr<IEntity> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const Registry& reg   = env.registry();
		std::string name	  = reg.getForObject<std::string>(RG_ENTITY, uuid, "name", "__unnamed__");
		std::string mesh_name = reg.getForObject<std::string>(RG_ENTITY, uuid, "mesh", "");

		if (!env.hasMesh(mesh_name))
			return nullptr;
		else
			return std::make_shared<MeshEntity>(id, name, env.getMesh(mesh_name));
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "mesh" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::MeshEntityFactory, "ent_mesh", PR_PLUGIN_VERSION)