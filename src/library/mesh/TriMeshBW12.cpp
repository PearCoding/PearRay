#include "TriMeshBW12.h"
#include "Profiler.h"
#include "container/kdTreeCollider.h"
#include "geometry/GeometryPoint.h"
#include "geometry/TriangleIntersection_BW.h"
#include "math/Tangent.h"
#include "mesh/MeshBase.h"

namespace PR {
struct TriMeshBW12Internal {
	size_t MatrixAttrib;
};

TriMeshBW12::TriMeshBW12(const std::string& name,
						 std::unique_ptr<MeshBase>&& mesh_base,
						 const std::shared_ptr<Cache>& cache,
						 bool useCache)
	: Mesh(name, std::move(mesh_base), cache, useCache)
	, mInternal(std::make_unique<TriMeshBW12Internal>())
{
	setup();
}

TriMeshBW12::~TriMeshBW12()
{
}

void TriMeshBW12::checkCollisionLocal(const RayPackage& in, CollisionOutput& out)
{
	std::vector<float>& matrices = mBase->userFaceAttrib(mInternal->MatrixAttrib);

	PR_ASSERT(mCollider && mBase, "Load before collision checks");
	mCollider
		->checkCollisionIncoherent(in, out,
								   [&](const RayPackage& in2, uint64 f, CollisionOutput& out2) {
									   Vector2fv uv;
									   vfloat t;

									   const float* M = &matrices[12 * f];

									   out2.Successful = TriangleIntersection::intersectBW12(
										   in2, M,
										   uv, out2.HitDistance);

									   out2.Parameter[0] = uv(0);
									   out2.Parameter[1] = uv(1);
									   out2.Parameter[2] = simdpp::make_zero();
									   out2.MaterialID	 = simdpp::make_uint(mBase->materialSlot(f)); // Has to be updated in entity!

									   out2.FaceID = simdpp::make_uint(f);
									   //out2.EntityID; Ignore
								   });
}

void TriMeshBW12::checkCollisionLocal(const Ray& in, SingleCollisionOutput& out)
{
	std::vector<float>& matrices = mBase->userFaceAttrib(mInternal->MatrixAttrib);

	PR_ASSERT(mCollider && mBase, "Load before collision checks");
	mCollider
		->checkCollisionSingle(in, out,
							   [&](const Ray& in2, uint64 f, SingleCollisionOutput& out2) {
								   Vector2f uv;

								   const float* M = &matrices[12 * f];

								   out2.Successful = TriangleIntersection::intersectBW12(
									   in2, M,
									   uv, out2.HitDistance);

								   out2.Parameter[0] = uv(0);
								   out2.Parameter[1] = uv(1);
								   out2.Parameter[2] = 0;

								   out2.MaterialID = mBase->materialSlot(f); // Has to be updated in entity!
								   out2.FaceID	   = static_cast<uint32>(f); // TODO: Maybe change to 64bit?
																			 //out2.EntityID; Ignore
							   });
}

void TriMeshBW12::setup()
{
	static const char* ATTRIB_MATRIX = "bw12_matrix";

	bool found;
	mInternal->MatrixAttrib = mBase->userFaceID(ATTRIB_MATRIX, found);
	if (found)
		return;

	const size_t facecount = mBase->faceCount();
	std::vector<float> matrices(12 * facecount);

	for (size_t f = 0; f < facecount; ++f) {
		const uint32 ind1 = mBase->indices()[3 * f + 0];
		const uint32 ind2 = mBase->indices()[3 * f + 1];
		const uint32 ind3 = mBase->indices()[3 * f + 2];

		const Vector3f p0 = mBase->vertex(ind1);
		const Vector3f p1 = mBase->vertex(ind2);
		const Vector3f p2 = mBase->vertex(ind3);

		TriangleIntersection::constructBW12Matrix(p0, p1, p2, &matrices[12 * f]);
	}

	mInternal->MatrixAttrib = mBase->addUserFaceAttrib(ATTRIB_MATRIX, matrices, 12);
}

} // namespace PR
