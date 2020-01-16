#include "Mesh.h"
#include "Logger.h"
#include "Platform.h"
#include "PrettyPrint.h"
#include "Profiler.h"
#include "TriangleOptions.h"
#include "container/kdTreeBuilder.h"
#include "container/kdTreeBuilderNaive.h"
#include "container/kdTreeCollider.h"
#include "geometry/GeometryPoint.h"
#include "math/Tangent.h"
#include "mesh/MeshBase.h"
#include "sampler/SplitSample.h"
#include "serialization/FileSerializer.h"

#include <fstream>

#define BUILDER kdTreeBuilder

namespace PR {
Mesh::Mesh(const std::string& name,
		   std::unique_ptr<MeshBase>&& mesh_base,
		   const std::shared_ptr<Cache>& cache,
		   bool useCache)
	: ISerializeCachable(name, cache, useCache)
	, mBase(std::move(mesh_base))
{
	setup();
}

Mesh::~Mesh()
{
}

float Mesh::surfaceArea(uint32 id)
{
	return doWork([&]() { return mBase->surfaceArea(id, Eigen::Affine3f::Identity()); });
}

bool Mesh::isCollidable() const
{
	return true;
}

float Mesh::collisionCost() const
{
	return static_cast<float>(mInfo.faceCount());
}

void Mesh::serialize(Serializer& serializer)
{
	try {
		mBase->serialize(serializer);
	} catch (const std::bad_alloc& ex) {
		PR_LOG(L_FATAL) << "Out of memory to load mesh " << name() << ": " << ex.what() << std::endl;
	}
}

void Mesh::beforeLoad()
{
	loadCollider();
	mBase = std::make_unique<MeshBase>();
}

void Mesh::afterLoad()
{
	PR_ASSERT(mBase, "Expect base to be load!");
	mInfo = mBase->info();
}

void Mesh::afterUnload()
{
	mCollider.reset();
	mBase.reset();
}

BoundingBox Mesh::localBoundingBox() const
{
	return mBoundingBox;
}

void Mesh::checkCollision(const RayPackage& in, CollisionOutput& out)
{
	doWork([&]() {
		PR_ASSERT(mBase, "Expect base to be load!");
		PR_ASSERT(mCollider, "Expect collider to be load!");

		checkCollisionLocal(in, out);
	});
}

void Mesh::checkCollision(const Ray& in, SingleCollisionOutput& out)
{
	doWork([&]() {
		PR_ASSERT(mBase, "Expect base to be load!");
		PR_ASSERT(mCollider, "Expect collider to be load!");

		checkCollisionLocal(in, out);
	});
}

Vector3f Mesh::pickRandomParameterPoint(const Vector2f& rnd, uint32& faceID, float& pdf)
{
	PR_PROFILE_THIS;

	SplitSample2D split(rnd, 0, mInfo.faceCount());
	faceID = split.integral1();

	Face face = doWork([&]() { return mBase->getFace(faceID); });
	pdf		  = 1.0f / (mInfo.faceCount() * face.surfaceArea());

	return Vector3f(rnd(0), rnd(1), 0);
}

void Mesh::provideGeometryPoint(uint32 faceID, const Vector3f& parameter, GeometryPoint& pt)
{
	Face f = doWork([&]() { return mBase->getFace(faceID); });

	Vector2f uv;
	f.interpolate(Vector2f(parameter[0], parameter[1]), pt.P, pt.N, uv);

	if (mInfo.Features & MF_HAS_UV)
		f.tangentFromUV(pt.N, pt.Nx, pt.Ny);
	else
		Tangent::frame(pt.N, pt.Nx, pt.Ny);

	pt.UVW = Vector3f(uv(0), uv(1), 0);
}

void Mesh::setup()
{
	mInfo = mBase->info();
	constructCollider();
	loadCollider();
}

void Mesh::constructCollider()
{
	PR_LOG(L_DEBUG) << "Mesh " << name() << " Memory Footprint: " << PR_FMT_MEM(mBase->memoryFootprint()) << std::endl;

	BUILDER builder(mBase.get(), [](void* observer, size_t f) {
								MeshBase* mesh = reinterpret_cast<MeshBase*>(observer);
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

#ifdef PR_DEBUG
	constexpr bool withStat = true;
#else
	constexpr bool withStat = false;
#endif

	builder.build(mBase->faceCount(), withStat);

	std::wstring cnt_file = this->cacheFileNoExt() + L".cnt";
	FileSerializer serializer(cnt_file, false);
	builder.save(serializer);

#ifdef PR_DEBUG
	PR_LOG(L_DEBUG) << "Mesh " << name() << " KDtree [depth="
					<< builder.depth()
					<< ", elements=" << mBase->faceCount()
					<< ", leafs=" << builder.leafCount()
					<< ", elementsPerLeaf=[avg:" << builder.avgElementsPerLeaf()
					<< ", min:" << builder.minElementsPerLeaf()
					<< ", max:" << builder.maxElementsPerLeaf()
					<< ", ET:" << builder.expectedTraversalSteps()
					<< ", EL:" << builder.expectedLeavesVisited()
					<< ", EI:" << builder.expectedObjectsIntersected()
					<< "]]" << std::endl;
#endif
}

void Mesh::loadCollider()
{
	std::wstring cnt_file = this->cacheFileNoExt() + L".cnt";
	FileSerializer serializer(cnt_file, true);
	mCollider = std::make_unique<kdTreeCollider>();
	mCollider->load(serializer);
	if (!mCollider->isEmpty())
		mBoundingBox = mCollider->boundingBox();
}

} // namespace PR
