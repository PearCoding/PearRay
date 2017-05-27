#include "BoundaryEntity.h"
#include "geometry/Plane.h"
#include "ray/Ray.h"
#include "shader/FaceSample.h"

#include "material/Material.h"
#include "math/Projection.h"
#include "sampler/Sampler.h"

#include "performance/Performance.h"

namespace PR {
BoundaryEntity::BoundaryEntity(uint32 id, const std::string& name, const BoundingBox& box)
	: RenderEntity(id, name)
	, mBoundingBox(box)
	, mMaterial(nullptr)
{
}

BoundaryEntity::~BoundaryEntity()
{
}

std::string BoundaryEntity::type() const
{
	return "boundary";
}

bool BoundaryEntity::isLight() const
{
	return mMaterial ? mMaterial->isLight() : false;
}

float BoundaryEntity::surfaceArea(Material* m) const
{
	PR_GUARD_PROFILE();

	if (!m || m == mMaterial.get()) {
		if (flags() & EF_LocalArea)
			return localBoundingBox().surfaceArea();
		else
			return worldBoundingBox().surfaceArea();
	} else {
		return 0;
	}
}

void BoundaryEntity::setMaterial(const std::shared_ptr<Material>& m)
{
	mMaterial = m;
}

const std::shared_ptr<Material>& BoundaryEntity::material() const
{
	return mMaterial;
}

void BoundaryEntity::setBoundingBox(const BoundingBox& box)
{
	mBoundingBox = box;
}

bool BoundaryEntity::isCollidable() const
{
	return mMaterial && mMaterial->canBeShaded() && mBoundingBox.isValid();
}

float BoundaryEntity::collisionCost() const
{
	return 2;
}

BoundingBox BoundaryEntity::localBoundingBox() const
{
	return mBoundingBox;
}

bool BoundaryEntity::checkCollision(const Ray& ray, FaceSample& collisionPoint) const
{
	PR_GUARD_PROFILE();

	Eigen::Vector3f vertex(0, 0, 0);

	Ray local = ray;
	local.setStartPosition(invTransform() * ray.startPosition());
	local.setDirection((invDirectionMatrix() * ray.direction()).normalized());

	BoundingBox box = localBoundingBox();
	float t;
	BoundingBox::FaceSide side;
	if (box.intersects(local, vertex, t, side)) {
		collisionPoint.P = transform() * vertex;

		Plane plane		  = box.getFace(side);
		collisionPoint.Ng = (directionMatrix() * plane.normal()).normalized();
		Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

		float u, v;
		plane.project(vertex, u, v);
		collisionPoint.UVW		= Eigen::Vector3f(u, v, 0);
		collisionPoint.Material = material().get();
		return true;
	}
	return false;
}

FaceSample BoundaryEntity::getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const
{
	PR_GUARD_PROFILE();

	auto ret = sampler.generate3D(sample);

	// Get randomly a face
	BoundingBox::FaceSide side = (BoundingBox::FaceSide)Projection::map(ret(0), 0, 5);
	Plane plane				   = localBoundingBox().getFace(side);

	FaceSample fp;
	fp.P  = transform() * (plane.xAxis() * ret(1) + plane.yAxis() * ret(2));
	fp.Ng = (directionMatrix() * plane.normal()).normalized();
	Projection::tangent_frame(fp.Ng, fp.Nx, fp.Ny);
	fp.UVW		= Eigen::Vector3f(ret(1), ret(2), 0);
	fp.Material = material().get();

	pdf = 1;
	return fp;
}

// Entity
void BoundaryEntity::setup(RenderContext* context)
{
	RenderEntity::setup(context);

	if (mMaterial)
		mMaterial->setup(context);
}
}
