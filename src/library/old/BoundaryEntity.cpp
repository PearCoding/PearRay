#include "BoundaryEntity.h"
#include "geometry/Plane.h"
#include "ray/Ray.h"
#include "shader/FacePoint.h"

#include "material/Material.h"
#include "math/Projection.h"

namespace PR {
BoundaryEntity::BoundaryEntity(uint32 id, const std::string& name, const BoundingBox& box)
	: RenderEntity(id, name)
	, mBoundingBox(box)
	, mMaterial(nullptr)
	, mPDF_Cache()
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

std::shared_ptr<Material> BoundaryEntity::material() const
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
	Ray local = ray;
	local.setOrigin(invTransform() * ray.origin());
	local.setDirection((invDirectionMatrix() * ray.direction()).normalized());

	RenderEntity::Collision c;
	BoundingBox box				 = localBoundingBox();
	BoundingBox::Intersection in = box.intersects(local);
	c.Successful				 = in.Successful;

	if (in.Successful) {
		c.Point.P = transform() * in.Position;

		Plane plane = box.getFace(box.getIntersectionSide(in));
		c.Point.Ng  = (directionMatrix() * plane.normal()).normalized();
		Projection::tangent_frame(c.Point.Ng, c.Point.Nx, c.Point.Ny);

		Eigen::Vector2f uv = plane.project(in.Position);
		c.Point.UVW		   = Eigen::Vector3f(uv(0), uv(1), 0);
		c.Point.Material   = material().get();
		return c;
	}

	return c;
}

void BoundaryEntity::checkCollisionV(const CollisionInput& in, CollisionOutput& out) const
{
	worldBoundingBox().intersectsV(in, out);
	out.EntityID   = simdpp::make_uint(id());
	out.FaceID	 = simdpp::make_uint(0);
	out.MaterialID = simdpp::make_uint(mMaterial->id());

	// TODO: UV
}

RenderEntity::FacePointSample BoundaryEntity::sampleFacePoint(const Eigen::Vector3f& rnd) const
{
	// Get random face
	BoundingBox::FaceSide side = (BoundingBox::FaceSide)Projection::map(rnd(0), 0, 5);
	Plane plane				   = localBoundingBox().getFace(side);

	RenderEntity::FacePointSample r;
	r.Point.P  = transform() * (plane.xAxis() * rnd(1) + plane.yAxis() * rnd(2));
	r.Point.Ng = (directionMatrix() * plane.normal()).normalized();
	Projection::tangent_frame(r.Point.Ng, r.Point.Nx, r.Point.Ny);
	r.Point.UVW		 = Eigen::Vector3f(rnd(1), rnd(2), 0);
	r.Point.Material = material().get();

	r.PDF_A = mPDF_Cache[(int)side];
	return r;
}

// VirtualEntity
void BoundaryEntity::onFreeze(RenderContext* context)
{
	RenderEntity::onFreeze(context);

	constexpr float prefactor = 1.0f / 6;

	for (int i = 0; i < 6; ++i) {
		Plane plane		 = worldBoundingBox().getFace((BoundingBox::FaceSide)i);
		const float area = plane.surfaceArea();
		mPDF_Cache[i]	= prefactor * (area > PR_EPSILON ? 1.0f / area : 0);
	}

	if (mMaterial)
		mMaterial->freeze(context);
}
} // namespace PR
