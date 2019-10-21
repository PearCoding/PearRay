#include "Environment.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "mesh/TriMesh.h"
#include "sampler/SplitSample.h"

namespace PR {

class MeshEntity : public IEntity {
public:
	ENTITY_CLASS

	MeshEntity(uint32 id, const std::string& name,
			   const std::shared_ptr<TriMesh>& mesh,
			   const std::vector<uint32>& materials,
			   int32 lightID)
		: IEntity(id, name)
		, mLightID(lightID)
		, mMaterials(materials)
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
		return mLightID >= 0;
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

		for (size_t i = 0; i < PR_SIMD_BANDWIDTH; ++i) {
			insert(i, out.MaterialID,
				   extract(i, out.MaterialID) < mMaterials.size()
					   ? mMaterials.at(extract(i, out.MaterialID))
					   : 0);
		}
	}

	void checkCollision(const Ray& in, SingleCollisionOutput& out) const override
	{
		auto in_local = in.transform(invTransform().matrix(), invDirectionMatrix());
		mMesh->checkCollision(in_local, out);

		// out.FaceID is set inside mesh
		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= id();
		out.MaterialID  = out.MaterialID < mMaterials.size()
							 ? mMaterials.at(out.MaterialID)
							 : 0;
	}

	Vector2f pickRandomPoint(const Vector2f& rnd, uint32& faceID, float& pdf) const override
	{
		SplitSample2D split(rnd, 0, mMesh->faceCount());
		faceID	= split.integral1();
		Face face = mMesh->getFace(split.integral1());

		pdf = 1.0f / (mMesh->faceCount() * face.surfaceArea());
		return Vector2f(rnd(0), rnd(1));
	}

	void provideGeometryPoint(uint32 faceID, float u, float v,
							  GeometryPoint& pt) const override
	{
		mMesh->provideGeometryPoint(faceID, u, v, pt);

		pt.P  = transform() * pt.P;
		pt.N  = directionMatrix() * pt.N;
		pt.Nx = directionMatrix() * pt.Nx;
		pt.Ny = directionMatrix() * pt.Ny;

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.MaterialID = pt.MaterialID < mMaterials.size()
							? mMaterials.at(pt.MaterialID)
							: 0;
		pt.EmissionID = mLightID;
		pt.DisplaceID = 0;
	}

protected:
	void onFreeze(RenderContext* context) override
	{
		IEntity::onFreeze(context);
	}

private:
	int32 mLightID;

	std::vector<uint32> mMaterials;
	std::shared_ptr<TriMesh> mMesh;
};

class MeshEntityFactory : public IEntityFactory {
public:
	std::shared_ptr<IEntity> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const Registry& reg   = env.registry();
		std::string name	  = reg.getForObject<std::string>(RG_ENTITY, uuid, "name", "__unnamed__");
		std::string mesh_name = reg.getForObject<std::string>(RG_ENTITY, uuid, "mesh", "");

		std::vector<std::string> matNames = reg.getForObject<std::vector<std::string>>(
			RG_ENTITY, uuid, "materials", std::vector<std::string>());

		std::vector<uint32> materials;
		for (std::string n : matNames) {
			std::shared_ptr<IMaterial> mat = env.getMaterial(n);
			if (mat)
				materials.push_back(mat->id());
		}

		std::string emsName = reg.getForObject<std::string>(
			RG_ENTITY, uuid, "emission", "");
		int32 emsID					   = -1;
		std::shared_ptr<IEmission> ems = env.getEmission(emsName);
		if (ems)
			emsID = ems->id();

		if (!env.hasMesh(mesh_name))
			return nullptr;
		else
			return std::make_shared<MeshEntity>(id, name, env.getMesh(mesh_name),
												materials, emsID);
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