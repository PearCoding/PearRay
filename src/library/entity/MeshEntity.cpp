#include "MeshEntity.h"

#include "geometry/Face.h"
#include "geometry/TriMesh.h"
#include "geometry/Triangle.h"
#include "material/Material.h"
#include "shader/FacePoint.h"

#include "math/Projection.h"

#include "Logger.h"

#include "performance/Performance.h"

namespace PR {
MeshEntity::MeshEntity(uint32 id, const std::string& name)
	: RenderEntity(id, name)
	, mMesh(nullptr)
{
}

MeshEntity::~MeshEntity()
{
}

std::string MeshEntity::type() const
{
	return "mesh";
}

bool MeshEntity::isLight() const
{
	for (auto ptr : mMaterials) {
		if (ptr->isLight())
			return true;
	}

	return false;
}

float MeshEntity::surfaceArea(Material* m) const
{
	if (isFrozen() && (m == nullptr)) {
		return mSurfaceArea_Cache;
	} else {
		if (m) {
			uint32 slot;
			for (slot = 0; slot < mMaterials.size(); ++slot) {
				if (mMaterials[slot].get() == m)
					break;
			}

			if (slot >= mMaterials.size())
				return mMesh->surfaceArea(flags() & EF_LocalArea ? Entity::Transform::Identity() : transform());
			else
				return mMesh->surfaceArea(slot, flags() & EF_LocalArea ? Entity::Transform::Identity() : transform());
		} else {
			return mMesh->surfaceArea(flags() & EF_LocalArea ? Entity::Transform::Identity() : transform());
		}
	}
}

void MeshEntity::setMesh(const std::shared_ptr<TriMesh>& mesh)
{
	mMesh = mesh;
}

const std::shared_ptr<TriMesh>& MeshEntity::mesh() const
{
	return mMesh;
}

void MeshEntity::reserveMaterialSlots(size_t count)
{
	mMaterials.resize(count);
}

void MeshEntity::setMaterial(uint32 slot, const std::shared_ptr<Material>& m)
{
	PR_ASSERT(slot < mMaterials.size(), "material slots not correctly reserved!");
	mMaterials[slot] = m;
}

std::shared_ptr<Material> MeshEntity::material(uint32 slot) const
{
	if (slot >= mMaterials.size())
		return mMaterials.front();
	else
		return mMaterials[slot];
}

bool MeshEntity::isCollidable() const
{
	if (!mMesh)
		return false;

	for (auto ptr : mMaterials) {
		if (ptr->canBeShaded())
			return true;
	}

	return false;
}

float MeshEntity::collisionCost() const
{
	return mMesh->collisionCost();
}

BoundingBox MeshEntity::localBoundingBox() const
{
	PR_ASSERT(mMesh, "mesh has to be initialized before accessing");
	return mMesh->boundingBox();
}

RenderEntity::Collision MeshEntity::checkCollision(const Ray& ray) const
{
	PR_GUARD_PROFILE();

	// Local space
	Ray local = ray;
	local.setOrigin(invTransform() * ray.origin());
	local.setDirection((invDirectionMatrix() * ray.direction()).normalized());

	TriMesh::Collision cm = mMesh->checkCollision(local);
	RenderEntity::Collision c;
	c.Successful = cm.Successful;

	if (cm.Successful) {
		c.Point.P  = transform() * cm.Point.P;
		c.Point.Ng = (directionMatrix() * cm.Point.Ng).normalized();
		Projection::tangent_frame(c.Point.Ng, c.Point.Nx, c.Point.Ny);

		c.Point.Material = material(cm.Ptr->MaterialSlot).get();

		return c;
	} else {
		return c;
	}
}

RenderEntity::FacePointSample MeshEntity::sampleFacePoint(const Eigen::Vector3f& rnd) const
{
	PR_GUARD_PROFILE();

	TriMesh::FacePointSample sm = mMesh->sampleFacePoint(rnd);
	RenderEntity::FacePointSample s;

	s.Point	= sm.Point;
	s.Point.Ng = (directionMatrix() * s.Point.Ng).normalized();
	s.Point.P  = transform() * s.Point.P;
	Projection::tangent_frame(s.Point.Ng, s.Point.Nx, s.Point.Ny);

	s.Point.Material = material(sm.MaterialSlot).get();
	s.PDF_A			 = sm.PDF;

	return s;
}

void MeshEntity::onFreeze()
{
	RenderEntity::onFreeze();

	mSurfaceArea_Cache = mMesh->surfaceArea(flags() & EF_LocalArea ? Entity::Transform::Identity() : transform());

	// Check up
	if (mSurfaceArea_Cache <= PR_EPSILON)
		PR_LOGGER.logf(L_Warning, M_Entity, "Mesh entity %s has zero surface area!", name().c_str());
}

void MeshEntity::setup(RenderContext* context)
{
	RenderEntity::setup(context);

	for (auto ptr : mMaterials)
		ptr->setup(context);
}

std::string MeshEntity::dumpInformation() const
{
	std::stringstream stream;
	stream << RenderEntity::dumpInformation()
		   << "    <MeshEntity>: " << std::endl
		   << "      Material Count: " << mMaterials.size() << std::endl;

	return stream.str();
}
}
