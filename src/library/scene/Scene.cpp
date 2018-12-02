#include "Scene.h"
#include "camera/CameraManager.h"
#include "camera/ICamera.h"
#include "container/kdTreeBuilder.h"
#include "container/kdTreeBuilderNaive.h"
#include "container/kdTreeCollider.h"
#include "emission/EmissionManager.h"
#include "emission/IEmission.h"
#include "entity/EntityManager.h"
#include "entity/IEntity.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/InfiniteLightManager.h"
#include "material/IMaterial.h"
#include "material/MaterialManager.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderManager.h"

#include "math/Compression.h"
#include "ray/RayStream.h"
#include "shader/HitStream.h"

#include "Logger.h"

#include <fstream>

#define BUILDER kdTreeBuilder

namespace PR {
Scene::Scene(const RenderManager* manager)
	: mRenderManager(manager)
	, mActiveCamera(manager->cameraManager()->getActiveCamera())
	, mKDTree(nullptr)
{
}

Scene::~Scene()
{
	if (mKDTree) {
		delete mKDTree;
		mKDTree = nullptr;
	}
}

std::shared_ptr<IEntity> Scene::getEntity(const std::string& name, const std::string& type) const
{
	return mRenderManager->entityManager()->getObjectByName(name, type);
}

const std::vector<std::shared_ptr<IEntity>>& Scene::entities() const
{

	return mRenderManager->entityManager()->getAll();
}

void Scene::buildTree(const std::string& file)
{
	size_t count = mRenderManager->entityManager()->size();
	PR_LOG(L_INFO) << count << " Entities" << std::endl;

	BUILDER builder(this,
					[](void* data, uint64 index) { return reinterpret_cast<Scene*>(data)->mRenderManager->entityManager()->getObject(index)->worldBoundingBox(); },
					[](void* data, uint64 index) {
						return reinterpret_cast<Scene*>(data)->mRenderManager->entityManager()->getObject(index)->collisionCost();
					},
					[](void* data, uint64 index, uint32 id) {
						reinterpret_cast<Scene*>(data)->mRenderManager->entityManager()->getObject(index)->setContainerID(id);
					});
	builder.setCostElementWise(true);
	builder.build(count);

	std::ofstream stream(file);
	builder.save(stream);
}

void Scene::loadTree(const std::string& file)
{
	if (mKDTree) {
		delete mKDTree;
		mKDTree = nullptr;
	}

	std::ifstream stream(file);
	mKDTree = new kdTreeCollider;
	mKDTree->load(stream);
	if (!mKDTree->isEmpty())
		mBoundingBox = mKDTree->boundingBox();
}

void Scene::traceCoherentRays(RayStream& rays, HitStream& hits) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");

	RayPackage in;
	CollisionOutput out;
	PR_SIMD_ALIGN float dir[3][PR_SIMD_BANDWIDTH];

	// First sort all rays
	rays.sort();

	// Split stream into specific groups
	// TODO: Currently only one group is present
	hits.reset();
	while (rays.hasNextGroup()) {
		RayGroup grp = rays.getNextGroup();

		// In some cases the group size will be not a multiply of the simd bandwith.
		// The internal stream is always a multiply therefore garbage may be traced
		// but no internal data will be corrupted.
		for (size_t i = 0;
			 i < grp.Size;
			 i += PR_SIMD_BANDWIDTH) {
			for (int j = 0; j < 3; ++j)
				in.Origin[j] = simdpp::load(&grp.Origin[j][i]);

			// Decompress
			for (size_t k = 0; k < PR_SIMD_BANDWIDTH; ++k) {
				from_oct(
					from_snorm16(grp.Direction[0][i + k]),
					from_snorm16(grp.Direction[1][i + k]),
					dir[0][k], dir[1][k], dir[2][k]);
			}

			for (int j = 0; j < 3; ++j)
				in.Direction[j] = simdpp::load(&dir[j][0]);

			in.setupInverse();

			// Check for collisions
			mKDTree
				->checkCollision(in, out,
								 [this](const RayPackage& in2, uint64 index, CollisionOutput& out2) {
									 mRenderManager->entityManager()->getObject(index)->checkCollision(in2, out2);
								 });
		}
	}

	// Clear ray stream, as it is finished
	rays.reset();
}

void Scene::traceIncoherentRays(RayStream& rays, HitStream& hits) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");

	Ray in;
	SingleCollisionOutput out;

	// First sort all rays
	rays.sort();

	// Split stream into specific groups
	// TODO: Currently only one group is present

	hits.reset();
	while (rays.hasNextGroup()) {
		RayGroup grp = rays.getNextGroup();

		for (size_t i = 0;
			 i < grp.Size;
			 ++i) {
			for (int j = 0; j < 3; ++j)
				in.Origin[j] = grp.Origin[j][i];

			// Decompress
			from_oct(
				from_snorm16(grp.Direction[0][i]),
				from_snorm16(grp.Direction[1][i]),
				in.Direction[0], in.Direction[1], in.Direction[2]);

			in.setupInverse();

			// Check for collisions
			mKDTree
				->checkCollision(in, out,
								 [this](const Ray& in2, uint64 index, SingleCollisionOutput& out2) {
									 mRenderManager->entityManager()->getObject(index)->checkCollision(in2, out2);
								 });
		}
	}

	// Clear ray stream, as it is finished
	rays.reset();
}

void Scene::traceShadowRays(RayStream& rays, HitStream& hits) const
{
	// TODO: Some special methods?
	traceIncoherentRays(rays, hits);
}

void Scene::setup(RenderContext* context)
{
	PR_LOG(L_INFO) << "Freezing scene" << std::endl;
	for (auto e : mRenderManager->emissionManager()->getAll())
		e->freeze(context);

	for (auto e : mRenderManager->materialManager()->getAll())
		e->freeze(context);

	for (auto e : mRenderManager->infiniteLightManager()->getAll())
		e->freeze(context);

	for (auto e : mRenderManager->entityManager()->getAll())
		e->freeze(context);

	for (auto e : mRenderManager->cameraManager()->getAll())
		e->freeze(context);

	const std::string file = context->renderManager()->workingDir() + "scene.cnt";
	PR_LOG(L_INFO) << "Starting to build global space-partitioning structure \"" << file << "\"" << std::endl;
	buildTree(file);
	loadTree(file);
}
} // namespace PR
