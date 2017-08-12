#include "CoordinateAxisEntity.h"
#include "geometry/Plane.h"
#include "ray/Ray.h"
#include "shader/FacePoint.h"

#include "material/Material.h"
#include "math/Projection.h"

#include "performance/Performance.h"

namespace PR {
CoordinateAxisEntity::CoordinateAxisEntity(uint32 id, const std::string& name)
	: RenderEntity(id, name)
	, mAxisLength(1)
	, mAxisThickness(0.05f)
	, mMaterials{ nullptr, nullptr, nullptr }
{
}

CoordinateAxisEntity::~CoordinateAxisEntity()
{
}

std::string CoordinateAxisEntity::type() const
{
	return "coordinate_axis";
}

bool CoordinateAxisEntity::isLight() const
{
	return (mMaterials[0] ? mMaterials[0]->isLight() : false) || (mMaterials[1] ? mMaterials[1]->isLight() : false) || (mMaterials[2] ? mMaterials[2]->isLight() : false);
}

float CoordinateAxisEntity::surfaceArea(Material* m) const
{
	PR_GUARD_PROFILE();

	if (!m) {
		if (flags() & EF_LocalArea)
			return localBoundingBox().surfaceArea();
		else
			return worldBoundingBox().surfaceArea();
	} else { // TODO: Add material specific surface area
		return 0;
	}
}

void CoordinateAxisEntity::setAxisLength(float f)
{
	mAxisLength = f;
}

float CoordinateAxisEntity::axisLength() const
{
	return mAxisLength;
}

void CoordinateAxisEntity::setAxisThickness(float f)
{
	mAxisThickness = f;
}

float CoordinateAxisEntity::axisThickness() const
{
	return mAxisThickness;
}

void CoordinateAxisEntity::setXMaterial(const std::shared_ptr<Material>& m)
{
	mMaterials[0] = m;
}

const std::shared_ptr<Material>& CoordinateAxisEntity::xMaterial() const
{
	return mMaterials[0];
}

void CoordinateAxisEntity::setYMaterial(const std::shared_ptr<Material>& m)
{
	mMaterials[1] = m;
}

const std::shared_ptr<Material>& CoordinateAxisEntity::yMaterial() const
{
	return mMaterials[1];
}

void CoordinateAxisEntity::setZMaterial(const std::shared_ptr<Material>& m)
{
	mMaterials[2] = m;
}

const std::shared_ptr<Material>& CoordinateAxisEntity::zMaterial() const
{
	return mMaterials[2];
}

bool CoordinateAxisEntity::isCollidable() const
{
	return mMaterials[0] && mMaterials[0]->canBeShaded() && mMaterials[1] && mMaterials[1]->canBeShaded() && mMaterials[2] && mMaterials[2]->canBeShaded() && mAxisLength > PR_EPSILON && mAxisThickness > PR_EPSILON;
}

float CoordinateAxisEntity::collisionCost() const
{
	return 6;
}

BoundingBox CoordinateAxisEntity::localBoundingBox() const
{
	if (!isFrozen())
		setup_cache();

	return mBoundingBox_Cache;
}

RenderEntity::Collision CoordinateAxisEntity::checkCollision(const Ray& ray) const
{
	PR_ASSERT(isFrozen(), "has to be frozen")
	PR_GUARD_PROFILE();

	Ray local = ray;
	local.setOrigin(invTransform() * ray.origin());
	local.setDirection((invDirectionMatrix() * ray.direction()).normalized());

	float t   = std::numeric_limits<float>::max();
	int found = -1;
	Eigen::Vector3f vertex;
	BoundingBox::FaceSide side;

	for (int i = 0; i < 3; ++i) {
		BoundingBox::Intersection in = mAxisBoundingBox_Cache[i].intersects(local);
		if (in.Successful && in.T > 0 && in.T < t) {
			t	  = in.T;
			found  = i;
			vertex = in.Position;
			side   = mAxisBoundingBox_Cache[i].getIntersectionSide(in);
		}
	}

	RenderEntity::Collision c;
	if (found >= 0) {
		PR_ASSERT(found < 3, "found can't be greater than 2");

		c.Point.P = transform() * vertex;

		Plane plane = mAxisBoundingBox_Cache[found].getFace(side);
		c.Point.Ng  = (directionMatrix() * plane.normal()).normalized();
		Projection::tangent_frame(c.Point.Ng, c.Point.Nx, c.Point.Ny);

		Eigen::Vector2f uv = plane.project(vertex);
		c.Point.UVW		 = Eigen::Vector3f(uv(0), uv(1), 0);
		c.Point.Material = mMaterials[found].get();
		c.Successful	 = true;
		return c;
	}

	c.Successful = false;
	return c;
}

RenderEntity::FacePointSample CoordinateAxisEntity::sampleFacePoint(const Eigen::Vector3f& rnd, uint32 sample) const
{
	PR_ASSERT(isFrozen(), "has to be frozen")
	PR_GUARD_PROFILE();

	int proj = Projection::map(rnd(0), 0, 3 * 6 - 1); // Get randomly a face

	int elem = proj / 3;
	PR_ASSERT(elem >= 0 && elem < 3, "elem has to be between 0 and 2");

	BoundingBox::FaceSide side = (BoundingBox::FaceSide)(proj % 6);
	Plane plane				   = mAxisBoundingBox_Cache[elem].getFace(side);

	RenderEntity::FacePointSample r;
	r.Point.P  = transform() * (plane.xAxis() * rnd(1) + plane.yAxis() * rnd(2));
	r.Point.Ng = (directionMatrix() * plane.normal()).normalized();
	Projection::tangent_frame(r.Point.Ng, r.Point.Nx, r.Point.Ny);
	r.Point.UVW		 = Eigen::Vector3f(rnd(1), rnd(2), 0);
	r.Point.Material = mMaterials[elem].get();

	r.PDF = 1;
	return r;
}

void CoordinateAxisEntity::setup_cache() const
{
	mAxisBoundingBox_Cache[0] = BoundingBox(
		Eigen::Vector3f(mAxisThickness, 0, 0),
		Eigen::Vector3f(mAxisLength, mAxisThickness, mAxisThickness));
	mAxisBoundingBox_Cache[1] = BoundingBox(
		Eigen::Vector3f(0, mAxisThickness, 0),
		Eigen::Vector3f(mAxisThickness, mAxisLength, mAxisThickness));
	mAxisBoundingBox_Cache[2] = BoundingBox(
		Eigen::Vector3f(0, 0, mAxisThickness),
		Eigen::Vector3f(mAxisThickness, mAxisThickness, mAxisLength));

	mBoundingBox_Cache = mAxisBoundingBox_Cache[0];
	mBoundingBox_Cache.combine(mAxisBoundingBox_Cache[1]);
	mBoundingBox_Cache.combine(mAxisBoundingBox_Cache[2]);
}

// Entity
void CoordinateAxisEntity::onFreeze()
{
	RenderEntity::onFreeze();
	setup_cache();
}

void CoordinateAxisEntity::setup(RenderContext* context)
{
	RenderEntity::setup(context);

	for (int i = 0; i < 3; ++i) {
		if (mMaterials[i])
			mMaterials[i]->setup(context);
	}
}
}
