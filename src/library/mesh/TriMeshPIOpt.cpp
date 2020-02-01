#include "TriMeshPIOpt.h"
#include "Profiler.h"
#include "container/kdTreeCollider.h"
#include "geometry/GeometryPoint.h"
#include "geometry/TriangleIntersection_PI.h"
#include "math/Tangent.h"
#include "mesh/MeshBase.h"

namespace PR {
struct TriMeshPIOptInternal {
	size_t FaceMomentumAttrib[3];
};

TriMeshPIOpt::TriMeshPIOpt(const std::string& name,
						   std::unique_ptr<MeshBase>&& mesh_base,
						   const std::shared_ptr<Cache>& cache,
						   bool useCache)
	: Mesh(name, std::move(mesh_base), cache, useCache)
	, mInternal(std::make_unique<TriMeshPIOptInternal>())
{
	setup();
}

TriMeshPIOpt::~TriMeshPIOpt()
{
}

void TriMeshPIOpt::checkCollisionLocal(const RayPackage& in, CollisionOutput& out)
{
	std::vector<float>& faceMomentum0 = mBase->userFaceAttrib(mInternal->FaceMomentumAttrib[0]);
	std::vector<float>& faceMomentum1 = mBase->userFaceAttrib(mInternal->FaceMomentumAttrib[1]);
	std::vector<float>& faceMomentum2 = mBase->userFaceAttrib(mInternal->FaceMomentumAttrib[2]);

	PR_ASSERT(mCollider && mBase, "Load before collision checks");
	mCollider
		->checkCollisionIncoherent(in, out,
								   [&](const RayPackage& in2, uint64 f, CollisionOutput& out2) {
									   Vector2fv uv;
									   vfloat t;

									   const uint32 ind1 = mBase->indices()[3 * f];
									   const uint32 ind2 = mBase->indices()[3 * f + 1];
									   const uint32 ind3 = mBase->indices()[3 * f + 2];

									   const Vector3fv p0 = promote(mBase->vertex(ind1));
									   const Vector3fv p1 = promote(mBase->vertex(ind2));
									   const Vector3fv p2 = promote(mBase->vertex(ind3));

									   const Vector3f m0 = Vector3f(faceMomentum0[3 * f + 0],
																	faceMomentum0[3 * f + 1],
																	faceMomentum0[3 * f + 2]);
									   const Vector3f m1 = Vector3f(faceMomentum1[3 * f + 0],
																	faceMomentum1[3 * f + 1],
																	faceMomentum1[3 * f + 2]);
									   const Vector3f m2 = Vector3f(faceMomentum2[3 * f + 0],
																	faceMomentum2[3 * f + 1],
																	faceMomentum2[3 * f + 2]);

									   bfloat hits = TriangleIntersection::intersectPI_Opt(
										   in2, p0, p1, p2,
										   promote(m0), promote(m1), promote(m2),
										   uv, t);

									   out2.HitDistance = simdpp::blend(t, out2.HitDistance, hits);

									   out2.Parameter[0] = uv(0);
									   out2.Parameter[1] = uv(1);
									   out2.Parameter[2] = simdpp::make_zero();
									   out2.MaterialID	 = simdpp::make_uint(mBase->materialSlot(f)); // Has to be updated in entity!

									   out2.FaceID = simdpp::make_uint(f);
									   //out2.EntityID; Ignore
								   });
}

void TriMeshPIOpt::checkCollisionLocal(const Ray& in, SingleCollisionOutput& out)
{
	std::vector<float>& faceMomentum0 = mBase->userFaceAttrib(mInternal->FaceMomentumAttrib[0]);
	std::vector<float>& faceMomentum1 = mBase->userFaceAttrib(mInternal->FaceMomentumAttrib[1]);
	std::vector<float>& faceMomentum2 = mBase->userFaceAttrib(mInternal->FaceMomentumAttrib[2]);

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

								   float t;
								   Vector2f uv;

								   const Vector3f m0 = Vector3f(faceMomentum0[3 * f + 0],
																faceMomentum0[3 * f + 1],
																faceMomentum0[3 * f + 2]);
								   const Vector3f m1 = Vector3f(faceMomentum1[3 * f + 0],
																faceMomentum1[3 * f + 1],
																faceMomentum1[3 * f + 2]);
								   const Vector3f m2 = Vector3f(faceMomentum2[3 * f + 0],
																faceMomentum2[3 * f + 1],
																faceMomentum2[3 * f + 2]);

								   bool hit = TriangleIntersection::intersectPI_Opt(
									   in2, p0, p1, p2,
									   m0, m1, m2,
									   uv, t);

								   if (!hit)
									   return;
								   else
									   out2.HitDistance = t;

								   out2.Parameter[0] = uv(0);
								   out2.Parameter[1] = uv(1);
								   out2.Parameter[2] = 0;

								   out2.MaterialID = mBase->materialSlot(f); // Has to be updated in entity!
								   out2.FaceID	   = static_cast<uint32>(f); // TODO: Maybe change to 64bit?
																			 //out2.EntityID; Ignore
							   });
}

void TriMeshPIOpt::setup()
{
	static const char* ATTRIB_MOMENTUM_0 = "pi_opt_momentum_0";
	static const char* ATTRIB_MOMENTUM_1 = "pi_opt_momentum_1";
	static const char* ATTRIB_MOMENTUM_2 = "pi_opt_momentum_2";

	bool found;
	mInternal->FaceMomentumAttrib[0] = mBase->userFaceID(ATTRIB_MOMENTUM_0, found);
	if (found)
		mInternal->FaceMomentumAttrib[1] = mBase->userFaceID(ATTRIB_MOMENTUM_1, found);
	if (found)
		mInternal->FaceMomentumAttrib[2] = mBase->userFaceID(ATTRIB_MOMENTUM_2, found);

	if (found)
		return;

	const size_t facecount = mBase->faceCount();
	std::vector<float> FaceMomentum0(3 * facecount);
	std::vector<float> FaceMomentum1(3 * facecount);
	std::vector<float> FaceMomentum2(3 * facecount);

	for (size_t f = 0; f < facecount; ++f) {
		const uint32 ind1 = mBase->indices()[3 * f + 0];
		const uint32 ind2 = mBase->indices()[3 * f + 1];
		const uint32 ind3 = mBase->indices()[3 * f + 2];

		const Vector3f p0 = mBase->vertex(ind1);
		const Vector3f p1 = mBase->vertex(ind2);
		const Vector3f p2 = mBase->vertex(ind3);

		// Cache momentum based on pluecker coordinates
		Vector3f m0 = p0.cross(p1);
		Vector3f m1 = p1.cross(p2);
		Vector3f m2 = p2.cross(p0);

		for (size_t i = 0; i < 3; ++i) {
			FaceMomentum0[3 * f + i] = m0(i);
			FaceMomentum1[3 * f + i] = m1(i);
			FaceMomentum2[3 * f + i] = m2(i);
		}
	}

	mInternal->FaceMomentumAttrib[0] = mBase->addUserFaceAttrib(ATTRIB_MOMENTUM_0, FaceMomentum0, 3);
	mInternal->FaceMomentumAttrib[1] = mBase->addUserFaceAttrib(ATTRIB_MOMENTUM_1, FaceMomentum1, 3);
	mInternal->FaceMomentumAttrib[2] = mBase->addUserFaceAttrib(ATTRIB_MOMENTUM_2, FaceMomentum2, 3);
}

} // namespace PR
