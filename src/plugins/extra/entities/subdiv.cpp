#include "Environment.h"
#include "Profiler.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"
#include "geometry/CollisionData.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "mesh/MeshContainer.h"
#include "sampler/SplitSample.h"

namespace PR {

class SubdivMeshEntity : public IEntity {
public:
	ENTITY_CLASS

	SubdivMeshEntity(uint32 id, const std::string& name,
					 const std::shared_ptr<MeshContainer>& mesh,
					 const std::vector<uint32>& materials,
					 int32 lightID)
		: IEntity(id, name)
		, mLightID(lightID)
		, mMaterials(materials)
		, mMesh(mesh)
	{
	}
	virtual ~SubdivMeshEntity() {}

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
		return mMesh->isValid();
	}

	float collisionCost() const override
	{
		// TODO
		return 0;
	}

	BoundingBox localBoundingBox() const override
	{
		// TODO
		return BoundingBox();
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		PR_PROFILE_THIS;
		// TODO
	}

	void checkCollision(const Ray& in, SingleCollisionOutput& out) const override
	{
		PR_PROFILE_THIS;
		// TODO
	}

	Vector2f pickRandomPoint(const Vector2f& rnd, uint32& faceID, float& pdf) const override
	{
		PR_PROFILE_THIS;

		SplitSample2D split(rnd, 0, mMesh->faceCount());
		faceID	= split.integral1();
		Face face = mMesh->getFace(split.integral1());

		pdf = 1.0f / (mMesh->faceCount() * face.surfaceArea());
		return Vector2f(rnd(0), rnd(1));
	}

	void provideGeometryPoint(uint32 faceID, float u, float v,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;
		// TODO
	}

protected:
	void onFreeze(RenderContext* context) override
	{
		IEntity::onFreeze(context);
	}

private:
	int32 mLightID;

	std::vector<uint32> mMaterials;
	std::shared_ptr<MeshContainer> mMesh;
};

class SubdivMeshEntityFactory : public IEntityFactory {
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

		// TODO
		return nullptr;
		/*if (!env.hasMesh(mesh_name))
			return nullptr;
		else
			return std::make_shared<SubdivMeshEntity>(id, name, env.getMesh(mesh_name),
													  materials, emsID);*/
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "subdiv", "subdivision", "opensubdiv" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SubdivMeshEntityFactory, "ent_subdiv", PR_PLUGIN_VERSION)