#include "TriMeshBW9.h"
#include "Profiler.h"
#include "container/kdTreeCollider.h"
#include "geometry/GeometryPoint.h"
#include "geometry/TriangleIntersection_BW.h"
#include "math/Tangent.h"
#include "mesh/MeshBase.h"

namespace PR {
struct TriMeshBW9Internal {
	size_t MatrixAttrib;
	size_t FixedColumnAttrib;
};

TriMeshBW9::TriMeshBW9(const std::string& name,
					   std::unique_ptr<MeshBase>&& mesh_base,
					   const std::shared_ptr<Cache>& cache,
					   bool useCache)
	: Mesh(name, std::move(mesh_base), cache, useCache)
	, mInternal(std::make_unique<TriMeshBW9Internal>())
{
	setup();
}

TriMeshBW9::~TriMeshBW9()
{
}

void TriMeshBW9::checkCollisionLocal(const RayPackage& in, CollisionOutput& out)
{
	std::vector<float>& matrices	 = mBase->userFaceAttrib(mInternal->MatrixAttrib);
	std::vector<uint8>& fixedColumns = mBase->userFaceAttribU8(mInternal->FixedColumnAttrib);

	PR_ASSERT(mCollider && mBase, "Load before collision checks");
	mCollider
		->checkCollisionIncoherent(in, out,
								   [&](const RayPackage& in2, uint64 f, CollisionOutput& out2) {
									   Vector2fv uv;
									   vfloat t;

									   const float* M = &matrices[9 * f];
									   const int FC	  = (int)fixedColumns[f];

									   out2.Successful = TriangleIntersection::intersectBW9(
										   in2, M, FC,
										   uv, out2.HitDistance);

									   out2.Parameter[0] = uv(0);
									   out2.Parameter[1] = uv(1);
									   out2.Parameter[2] = simdpp::make_zero();
									   out2.MaterialID	 = simdpp::make_uint(mBase->materialSlot(f)); // Has to be updated in entity!

									   out2.FaceID = simdpp::make_uint(f);
									   //out2.EntityID; Ignore
								   });
}

void TriMeshBW9::checkCollisionLocal(const Ray& in, SingleCollisionOutput& out)
{
	std::vector<float>& matrices	 = mBase->userFaceAttrib(mInternal->MatrixAttrib);
	std::vector<uint8>& fixedColumns = mBase->userFaceAttribU8(mInternal->FixedColumnAttrib);

	PR_ASSERT(mCollider && mBase, "Load before collision checks");
	mCollider
		->checkCollisionSingle(in, out,
							   [&](const Ray& in2, uint64 f, SingleCollisionOutput& out2) {
								   Vector2f uv;

								   const float* M = &matrices[9 * f];
								   const int FC	  = (int)fixedColumns[f];

								   out2.Successful = TriangleIntersection::intersectBW9(
									   in2, M, FC,
									   uv, out2.HitDistance);

								   out2.Parameter[0] = uv(0);
								   out2.Parameter[1] = uv(1);
								   out2.Parameter[2] = 0;

								   out2.MaterialID = mBase->materialSlot(f); // Has to be updated in entity!
								   out2.FaceID	   = static_cast<uint32>(f); // TODO: Maybe change to 64bit?
																			 //out2.EntityID; Ignore
							   });
}

void TriMeshBW9::setup()
{
	static const char* ATTRIB_MATRIX	   = "bw9_matrix";
	static const char* ATTRIB_FIXED_COLUMN = "bw9_fixed_column";

	bool found;
	mInternal->MatrixAttrib = mBase->userFaceID(ATTRIB_MATRIX, found);
	if (found)
		mInternal->FixedColumnAttrib = mBase->userFaceID(ATTRIB_FIXED_COLUMN, found);

	if (found)
		return;

	const size_t facecount = mBase->faceCount();
	std::vector<float> matrices(9 * facecount);
	std::vector<uint8> fixedColumns(facecount);

	for (size_t f = 0; f < facecount; ++f) {
		const uint32 ind1 = mBase->indices()[3 * f + 0];
		const uint32 ind2 = mBase->indices()[3 * f + 1];
		const uint32 ind3 = mBase->indices()[3 * f + 2];

		const Vector3f p0 = mBase->vertex(ind1);
		const Vector3f p1 = mBase->vertex(ind2);
		const Vector3f p2 = mBase->vertex(ind3);

		int fc;
		TriangleIntersection::constructBW9Matrix(p0, p1, p2, &matrices[9 * f], fc);
		fixedColumns[f] = (uint8)fc;
	}

	mInternal->MatrixAttrib		 = mBase->addUserFaceAttrib(ATTRIB_MATRIX, matrices, 9);
	mInternal->FixedColumnAttrib = mBase->addUserFaceAttribU8(ATTRIB_FIXED_COLUMN, fixedColumns, 1);
}

} // namespace PR
