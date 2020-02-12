#include "TriMeshPI.h"
#include "Profiler.h"
#include "container/kdTreeCollider.h"
#include "geometry/GeometryPoint.h"
#include "geometry/TriangleIntersection_PI.h"
#include "math/Tangent.h"
#include "mesh/MeshBase.h"

namespace PR {
TriMeshPI::TriMeshPI(const std::string& name,
					 std::unique_ptr<MeshBase>&& mesh_base,
					 const std::shared_ptr<Cache>& cache,
					 bool useCache)
	: Mesh(name, std::move(mesh_base), cache, useCache)
{
}

TriMeshPI::~TriMeshPI()
{
}

void TriMeshPI::checkCollisionLocal(const RayPackage& in, CollisionOutput& out)
{
	PR_ASSERT(mCollider && mBase, "Load before collision checks");
	mCollider
		->checkCollisionIncoherent(in, out,
								   [&](const RayPackage& in2, uint64 f, CollisionOutput& out2) {
									   Vector2fv uv;

									   const uint32 ind1 = mBase->indices()[3 * f];
									   const uint32 ind2 = mBase->indices()[3 * f + 1];
									   const uint32 ind3 = mBase->indices()[3 * f + 2];

									   const Vector3fv p0 = promote(mBase->vertex(ind1));
									   const Vector3fv p1 = promote(mBase->vertex(ind2));
									   const Vector3fv p2 = promote(mBase->vertex(ind3));

									   out2.Successful = TriangleIntersection::intersectPI(in2, p0, p1, p2, uv, out2.HitDistance);

									   out2.Parameter[0] = uv(0);
									   out2.Parameter[1] = uv(1);
									   out2.Parameter[2] = simdpp::make_zero();
									   out2.MaterialID	 = simdpp::make_uint(mBase->materialSlot(f)); // Has to be updated in entity!

									   out2.FaceID = simdpp::make_uint(f);
									   //out2.EntityID; Ignore
								   });
}

void TriMeshPI::checkCollisionLocal(const Ray& in, SingleCollisionOutput& out)
{
	PR_ASSERT(mCollider && mBase, "Load before collision checks");
	mCollider
		->checkCollisionSingle(in, out,
							   [&](const Ray& in2, uint64 f, SingleCollisionOutput& out2) {
								   const uint32 ind1 = mBase->indices()[3 * f];
								   const uint32 ind2 = mBase->indices()[3 * f + 1];
								   const uint32 ind3 = mBase->indices()[3 * f + 2];

								   const Vector3f p0 = mBase->vertex(ind1);
								   const Vector3f p1 = mBase->vertex(ind2);
								   const Vector3f p2 = mBase->vertex(ind3);

								   Vector2f uv;

								   out2.Successful = TriangleIntersection::intersectPI(in2, p0, p1, p2, uv, out2.HitDistance);

								   out2.Parameter[0] = uv(0);
								   out2.Parameter[1] = uv(1);
								   out2.Parameter[2] = 0;

								   out2.MaterialID = mBase->materialSlot(f); // Has to be updated in entity!
								   out2.FaceID	   = static_cast<uint32>(f); // TODO: Maybe change to 64bit?
																			 //out2.EntityID; Ignore
							   });
}

} // namespace PR
