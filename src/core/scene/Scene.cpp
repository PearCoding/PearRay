#include "Scene.h"
#include "Platform.h"
#include "camera/ICamera.h"
#include "container/kdTreeBuilder.h"
#include "container/kdTreeBuilderNaive.h"
#include "emission/IEmission.h"
#include "infinitelight/IInfiniteLight.h"
#include "material/IMaterial.h"
#include "renderer/RenderContext.h"
#include "serialization/FileSerializer.h"

#include "Logger.h"

#include <boost/filesystem.hpp>
#include <fstream>

#define BUILDER kdTreeBuilder

namespace PR {
Scene::Scene(const std::shared_ptr<ICamera>& activeCamera,
			 const std::vector<std::shared_ptr<IEntity>>& entities,
			 const std::vector<std::shared_ptr<IMaterial>>& materials,
			 const std::vector<std::shared_ptr<IEmission>>& emissions,
			 const std::vector<std::shared_ptr<IInfiniteLight>>& infLights,
			 const std::wstring& cntFile)
	: mActiveCamera(activeCamera)
	, mEntities(entities)
	, mMaterials(materials)
	, mEmissions(emissions)
	, mInfLights(infLights)
	, mKDTree(nullptr)
{
	PR_LOG(L_DEBUG) << "Setup before scene build..." << std::endl;
	mActiveCamera->beforeSceneBuild();
	for (auto o : mEmissions)
		o->beforeSceneBuild();
	for (auto o : mInfLights)
		o->beforeSceneBuild();
	for (auto o : mMaterials)
		o->beforeSceneBuild();
	for (auto o : mEntities)
		o->beforeSceneBuild();

	PR_LOG(L_DEBUG) << "Starting to build global space-partitioning structure " << boost::filesystem::path(cntFile) << std::endl;
	buildTree(cntFile);
	loadTree(cntFile);

	PR_LOG(L_DEBUG) << "Setup after scene build..." << std::endl;
	mActiveCamera->afterSceneBuild(this);
	for (auto o : mEmissions)
		o->afterSceneBuild(this);
	for (auto o : mInfLights)
		o->afterSceneBuild(this);
	for (auto o : mMaterials)
		o->afterSceneBuild(this);
	for (auto o : mEntities)
		o->afterSceneBuild(this);
}

Scene::~Scene()
{
}
void Scene::beforeRender(RenderContext* ctx)
{
	PR_LOG(L_DEBUG) << "Setup before render start..." << std::endl;

	mActiveCamera->beforeRender(ctx);
	for (auto o : mEmissions)
		o->beforeRender(ctx);
	for (auto o : mInfLights)
		o->beforeRender(ctx);
	for (auto o : mMaterials)
		o->beforeRender(ctx);
	for (auto o : mEntities)
		o->beforeRender(ctx);
}

void Scene::afterRender(RenderContext* ctx)
{
	PR_LOG(L_DEBUG) << "Setup after render stop..." << std::endl;

	mActiveCamera->afterRender(ctx);
	for (auto o : mEmissions)
		o->afterRender(ctx);
	for (auto o : mInfLights)
		o->afterRender(ctx);
	for (auto o : mMaterials)
		o->afterRender(ctx);
	for (auto o : mEntities)
		o->afterRender(ctx);
}

void Scene::buildTree(const std::wstring& file)
{
	PR_PROFILE_THIS;

	size_t count = mEntities.size();
	PR_LOG(L_INFO) << count << " Entities" << std::endl;

	BUILDER builder(
		this,
		[](void* data, size_t index) { return reinterpret_cast<Scene*>(data)->mEntities[index]->worldBoundingBox(); },
		[](void* data, size_t index) {
			return reinterpret_cast<Scene*>(data)->mEntities[index]->collisionCost();
		},
		[](void* data, size_t index, size_t id) {
			reinterpret_cast<Scene*>(data)->mEntities[index]->setContainerID(id);
		});
	builder.setCostElementWise(true);
	builder.build(count);

	FileSerializer serializer(file, false);
	if (!serializer.isValid()) {
		PR_LOG(L_ERROR) << "Could not open " << boost::filesystem::path(file) << " to write scene cnt" << std::endl;
		return;
	}
	builder.save(serializer);
}

void Scene::loadTree(const std::wstring& file)
{
	PR_PROFILE_THIS;

	mKDTree.reset(new kdTreeCollider);

	FileSerializer serializer(file, true);
	if (!serializer.isValid()) {
		PR_LOG(L_ERROR) << "Could not open " << boost::filesystem::path(file) << " to read scene cnt" << std::endl;
		return;
	}

	mKDTree->load(serializer);
	if (!mKDTree->isEmpty())
		mBoundingBox = mKDTree->boundingBox();
}

void Scene::traceRays(RayStream& rays, HitStream& hits) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");

	// Split stream into specific groups
	hits.reset();
	while (rays.hasNextGroup()) {
		RayGroup grp = rays.getNextGroup();

		if (grp.isCoherent())
			traceCoherentRays(grp, hits);
		else
			traceIncoherentRays(grp, hits);
	}
}

template <uint32 K>
inline void _sceneCheckHit(const RayGroup& grp,
						   uint32 off, const CollisionOutput& out,
						   HitStream& hits)
{
	const uint32 id = off + K;
	if (id >= grp.size()) // Ignore bad tails
		return;

	bool successful = simdpp::extract<K>(simdpp::bit_cast<vuint32::Base>(out.Successful));

	HitEntry entry;
	entry.RayID		  = id + grp.offset();
	entry.MaterialID  = successful ? simdpp::extract<K>(out.MaterialID) : PR_INVALID_ID;
	entry.EntityID	  = successful ? simdpp::extract<K>(out.EntityID) : PR_INVALID_ID;
	entry.PrimitiveID = successful ? simdpp::extract<K>(out.FaceID) : PR_INVALID_ID;
	for (int i = 0; i < 3; ++i)
		entry.Parameter[i] = simdpp::extract<K>(out.Parameter[i]);
	entry.Flags = simdpp::extract<K>(out.Flags);

	PR_ASSERT(!hits.isFull(), "Unbalanced hit and ray stream size!");
	hits.add(entry);
}

template <uint32 C>
class _sceneCheckHitCallee {
public:
	inline void operator()(const RayGroup& grp,
						   uint32 off, const CollisionOutput& out,
						   HitStream& hits)
	{
		_sceneCheckHit<C - 1>(grp, off, out, hits);
		_sceneCheckHitCallee<C - 1>()(grp, off, out, hits);
	}
};

template <>
class _sceneCheckHitCallee<0> {
public:
	inline void operator()(const RayGroup&,
						   uint32, const CollisionOutput&,
						   HitStream&)
	{
	}
};

void Scene::traceCoherentRays(const RayGroup& grp, HitStream& hits) const
{
	PR_PROFILE_THIS;

	RayPackage in;
	CollisionOutput out;

	// In some cases the group size will be not a multiply of the simd bandwith.
	// The internal stream is always a multiply therefore garbage may be traced
	// but no internal data will be corrupted.
	for (size_t i = 0;
		 i < grp.size();
		 i += PR_SIMD_BANDWIDTH) {
		in = grp.getRayPackage(i);

		// Check for collisions
		mKDTree->checkCollisionCoherent(
			in, out,
			[this](const RayPackage& in2, uint64 index,
				   CollisionOutput& out2) {
				const IEntity* entity = mEntities[index].get();
				entity->checkCollision(in2, out2);
				out2.Successful = out2.Successful & ((vuint32(entity->visibilityFlags()) & in2.Flags) != vuint32(0));
			});

		_sceneCheckHitCallee<PR_SIMD_BANDWIDTH>()(grp, i, out, hits);
	}
}

void Scene::traceIncoherentRays(const RayGroup& grp, HitStream& hits) const
{
	PR_PROFILE_THIS;
	RayPackage in;
	CollisionOutput out;

	// In some cases the group size will be not a multiply of the simd bandwith.
	// The internal stream is always a multiply therefore garbage may be traced
	// but no internal data will be corrupted.
	for (size_t i = 0;
		 i < grp.size();
		 i += PR_SIMD_BANDWIDTH) {
		in = grp.getRayPackage(i);

		// Check for collisions
		mKDTree->checkCollisionIncoherent(
			in, out,
			[this](const RayPackage& in2, uint64 index,
				   CollisionOutput& out2) {
				const IEntity* entity = mEntities[index].get();
				entity->checkCollision(in2, out2);
				out2.Successful = out2.Successful & ((vuint32(entity->visibilityFlags()) & in2.Flags) != vuint32(0));
			});

		_sceneCheckHitCallee<PR_SIMD_BANDWIDTH>()(grp, i, out, hits);
	}
}
} // namespace PR
