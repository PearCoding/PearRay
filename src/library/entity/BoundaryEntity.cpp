#include "BoundaryEntity.h"
#include "geometry/Plane.h"
#include "ray/Ray.h"
#include "shader/FacePoint.h"

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

RenderEntity::Collision BoundaryEntity::checkCollision(const Ray& ray) const
{
	PR_GUARD_PROFILE();

	Eigen::Vector3f vertex(0, 0, 0);

	Ray local = ray;
	local.setOrigin(invTransform() * ray.origin());
	local.setDirection((invDirectionMatrix() * ray.direction()).normalized());

	RenderEntity::Collision c;
	BoundingBox box = localBoundingBox();
	BoundingBox::Intersection in = box.intersects(local);
	c.Successful = in.Successful;

	if (in.Successful) {
		c.Point.P = transform() * in.Position;

		Plane plane = box.getFace(box.getIntersectionSide(in));
		c.Point.Ng  = (directionMatrix() * plane.normal()).normalized();
		Projection::tangent_frame(c.Point.Ng, c.Point.Nx, c.Point.Ny);

		Eigen::Vector2f uv = plane.project(in.Position);
		c.Point.UVW		 = Eigen::Vector3f(uv(0), uv(1), 0);
		c.Point.Material = material().get();
		return c;
	}

	return c;
}

RenderEntity::FacePointSample BoundaryEntity::sampleFacePoint(Sampler& sampler, uint32 sample) const
{
	PR_GUARD_PROFILE();

	auto ret = sampler.generate3D(sample);

	// Get randomly a face
	BoundingBox::FaceSide side = (BoundingBox::FaceSide)Projection::map(ret(0), 0, 5);
	Plane plane				   = localBoundingBox().getFace(side);

	RenderEntity::FacePointSample r;
	r.Point.P  = transform() * (plane.xAxis() * ret(1) + plane.yAxis() * ret(2));
	r.Point.Ng = (directionMatrix() * plane.normal()).normalized();
	Projection::tangent_frame(r.Point.Ng, r.Point.Nx, r.Point.Ny);
	r.Point.UVW		= Eigen::Vector3f(ret(1), ret(2), 0);
	r.Point.Material = material().get();

	r.PDF = 1;
	return r;
}

// Entity
void BoundaryEntity::setup(RenderContext* context)
{
	RenderEntity::setup(context);

	if (mMaterial)
		mMaterial->setup(context);
}
}
