#include "CacheManager.h"
#include "Environment.h"
#include "Logger.h"
#include "Platform.h"
#include "Profiler.h"
#include "container/kdTreeBuilder.h"
#include "container/kdTreeBuilderNaive.h"
#include "container/kdTreeCollider.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"
#include "geometry/CollisionData.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "mesh/MeshContainer.h"
#include "sampler/SplitSample.h"

#include <boost/filesystem.hpp>

#define BUILDER kdTreeBuilder

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
		return mMesh->surfaceArea(id, transform());
	}

	bool isCollidable() const override
	{
		return mMesh->isValid() && mKDTree;
	}

	float collisionCost() const override
	{
		return static_cast<float>(mMesh->faceCount());
	}

	BoundingBox localBoundingBox() const override
	{
		return mBoundingBox;
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());
		mKDTree
			->checkCollision(in_local, out,
							 [this](const RayPackage& in2, uint64 f, CollisionOutput& out2) {
								 Vector2fv uv;
								 vfloat t;

								 const uint32 ind1 = mMesh->indices()[3 * f];
								 const uint32 ind2 = mMesh->indices()[3 * f + 1];
								 const uint32 ind3 = mMesh->indices()[3 * f + 2];

								 const Vector3fv p0 = promote(mMesh->vertex(ind1));
								 const Vector3fv p1 = promote(mMesh->vertex(ind2));
								 const Vector3fv p2 = promote(mMesh->vertex(ind3));

								 bfloat hits = Triangle::intersect(
									 in2,
									 p0, p1, p2,
									 uv,
									 t); // Major bottleneck!

								 const vfloat inf = simdpp::make_float(std::numeric_limits<float>::infinity());
								 out2.HitDistance = simdpp::blend(t, inf, hits);

								 if (mMesh->features() & MF_HAS_UV) {
									 for (int i = 0; i < 2; ++i)
										 out2.UV[i] = mMesh->uvs(i)[ind2] * uv(0)
													  + mMesh->uvs(i)[ind3] * uv(1)
													  + mMesh->uvs(i)[ind1] * (1 - uv(0) - uv(1));
								 } else {
									 out2.UV[0] = uv(0);
									 out2.UV[1] = uv(1);
								 }

								 if (mMesh->features() & MF_HAS_MATERIAL)
									 out2.MaterialID = simdpp::make_uint(mMaterials[f]); // Has to be updated in entity!
								 else
									 out2.MaterialID = simdpp::make_uint(0);

								 out2.FaceID = simdpp::make_uint(f);
								 //out2.EntityID; Ignore
							 });

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
		mKDTree
			->checkCollision(in_local, out,
							 [this](const Ray& in2, uint64 f, SingleCollisionOutput& out2) {
								 const uint32 ind1 = mMesh->indices()[3 * f];
								 const uint32 ind2 = mMesh->indices()[3 * f + 1];
								 const uint32 ind3 = mMesh->indices()[3 * f + 2];

								 float t;
								 Vector2f uv;
								 bool hit = Triangle::intersect(
									 in2,
									 mMesh->vertex(ind1), mMesh->vertex(ind2), mMesh->vertex(ind3),
									 uv, t); // Major bottleneck!

								 if (!hit)
									 return;
								 else
									 out2.HitDistance = t;

								 if (mMesh->features() & MF_HAS_UV) {
									 auto v = Triangle::interpolate(
										 mMesh->uv(ind1), mMesh->uv(ind2), mMesh->uv(ind3),
										 uv);
									 out2.UV[0] = v(0);
									 out2.UV[1] = v(1);
								 } else {
									 out2.UV[0] = uv(0);
									 out2.UV[1] = uv(1);
								 }

								 if (mMesh->features() & MF_HAS_MATERIAL)
									 out2.MaterialID = mMaterials[f]; // Has to be updated in entity!
								 else
									 out2.MaterialID = 0;

								 out2.FaceID = f;
								 //out2.EntityID; Ignore
							 });

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

		Face f = mMesh->getFace(faceID);

		Vector2f local_uv;
		if (mMesh->features() & MF_HAS_UV)
			local_uv = f.mapGlobalToLocalUV(Vector2f(u, v));
		else
			local_uv = Vector2f(u, v);

		Vector2f uv;
		f.interpolate(local_uv, pt.P, pt.N, uv);

		if (mMesh->features() & MF_HAS_UV)
			f.tangentFromUV(pt.N, pt.Nx, pt.Ny);
		else
			Tangent::frame(pt.N, pt.Nx, pt.Ny);

		pt.UVW		  = Vector3f(uv(0), uv(1), 0);
		pt.MaterialID = f.MaterialSlot;

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

		mMesh->triangulate();
		if (!mLoadOnly)
			buildTree();

		loadTree();

		IEntity::beforeSceneBuild();
	}

private:
	void buildTree()
	{
		BUILDER builder(mMesh.get(), [](void* observer, size_t f) {
								MeshContainer* mesh = reinterpret_cast<MeshContainer*>(observer);
								const uint32 ind1 = mesh->indices()[3*f];
								const uint32 ind2 = mesh->indices()[3*f+1];
								const uint32 ind3 = mesh->indices()[3*f+2];

								Vector3f p1 = mesh->vertex(ind1);
								Vector3f p2 = mesh->vertex(ind2);
								Vector3f p3 = mesh->vertex(ind3);
								return Triangle::getBoundingBox(p1,p2,p3); },
						[](void*, size_t) {
							return 4.0f;
						});
		builder.build(mMesh->faceCount());

		std::ofstream stream(encodePath(mCNTFile), std::ios::out | std::ios::trunc);
		builder.save(stream);

		PR_LOG(L_INFO) << "Mesh KDtree [depth="
					   << builder.depth()
					   << ", elements=" << mMesh->faceCount()
					   << ", leafs=" << builder.leafCount()
					   << ", elementsPerLeaf=[avg:" << builder.avgElementsPerLeaf()
					   << ", min:" << builder.minElementsPerLeaf()
					   << ", max:" << builder.maxElementsPerLeaf()
					   << ", ET:" << builder.expectedTraversalSteps()
					   << ", EL:" << builder.expectedLeavesVisited()
					   << ", EI:" << builder.expectedObjectsIntersected()
					   << "]]" << std::endl;
	}

	void loadTree()
	{
		std::ifstream stream(encodePath(mCNTFile));
		mKDTree = std::make_unique<kdTreeCollider>();
		mKDTree->load(stream);
		if (!mKDTree->isEmpty())
			mBoundingBox = mKDTree->boundingBox();
	}

	std::wstring mCNTFile;
	bool mLoadOnly;

	BoundingBox mBoundingBox;
	std::unique_ptr<kdTreeCollider> mKDTree;

	int32 mLightID;

	std::vector<uint32> mMaterials;
	std::shared_ptr<MeshContainer> mMesh;
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