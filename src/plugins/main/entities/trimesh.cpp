#include "mesh/TriMesh.h"
#include "CacheManager.h"
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

class TriMeshEntity : public IEntity {
public:
	ENTITY_CLASS

	TriMeshEntity(uint32 id, const std::string& name,
				  const std::wstring& cnt_file, bool load_only,
				  const std::shared_ptr<MeshContainer>& mesh,
				  const std::vector<uint32>& materials,
				  int32 lightID)
		: IEntity(id, name)
		, mCNTFile(cnt_file)
		, mLoadOnly(load_only)
		, mLightID(lightID)
		, mMaterials(materials)
		, mMesh(mesh)
	{
	}
	virtual ~TriMeshEntity() {}

	std::string type() const override
	{
		return "trimesh";
	}

	bool isLight() const override
	{
		return mLightID >= 0;
	}

	float surfaceArea(uint32 id) const override
	{
		return mMesh.surfaceArea(id);
	}

	bool isCollidable() const override
	{
		return mMesh.isCollidable();
	}

	float collisionCost() const override
	{
		return mMesh.collisionCost();
	}

	BoundingBox localBoundingBox() const override
	{
		return mMesh.localBoundingBox();
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());
		mMesh.checkCollision(in_local, out);

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
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());
		mMesh.checkCollision(in_local, out);

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
		PR_PROFILE_THIS;
		return mMesh.pickRandomPoint(rnd, faceID, pdf);
	}

	void provideGeometryPoint(uint32 faceID, float u, float v,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		mMesh.provideGeometryPoint(faceID, u, v, pt);

		// Global
		pt.P  = transform() * pt.P;
		pt.N  = normalMatrix() * pt.N;
		pt.Nx = normalMatrix() * pt.Nx;
		pt.Ny = normalMatrix() * pt.Ny;

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.MaterialID = pt.MaterialID < mMaterials.size()
							? mMaterials.at(pt.MaterialID)
							: 0;
		pt.EmissionID = mLightID;
		pt.DisplaceID = 0;
	}

	void beforeSceneBuild() override
	{
		PR_LOG(L_INFO) << "Caching mesh " << name() << " [" << boost::filesystem::path(mCNTFile) << "]";

		if (!mLoadOnly)
			mMesh.build(mCNTFile);
		else
			mMesh.load(mCNTFile);

		IEntity::beforeSceneBuild();
	}

private:
	std::wstring mCNTFile;
	bool mLoadOnly;

	int32 mLightID;
	std::vector<uint32> mMaterials;
	TriMesh mMesh;
};

class TriMeshEntityFactory : public IEntityFactory {
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

		std::wstring customCNT = reg.getForObject<std::wstring>(RG_ENTITY, uuid, "cnt", L"");

		boost::filesystem::path path;
		bool load_only = false;
		if (customCNT.empty()) {
			path = env.cacheManager()->requestFile("mesh", name + ".cnt", load_only);
		} else {
			path	  = boost::filesystem::absolute(customCNT, env.workingDir());
			load_only = true;
		}

		if (!env.hasMesh(mesh_name))
			return nullptr;
		else {
			auto mesh = env.getMesh(mesh_name);
			if (!mesh->isValid()) {
				PR_LOG(L_ERROR) << "Mesh " << mesh_name << " is invalid." << std::endl;
				return nullptr;
			}

			return std::make_shared<TriMeshEntity>(id, name,
												   path.generic_wstring(), load_only,
												   mesh,
												   materials, emsID);
		}
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

PR_PLUGIN_INIT(PR::TriMeshEntityFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)