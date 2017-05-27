#include "CoordinateAxisEntity.h"
#include "geometry/Plane.h"
#include "ray/Ray.h"
#include "shader/FaceSample.h"

#include "material/Material.h"
#include "math/Projection.h"
#include "sampler/Sampler.h"

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

bool CoordinateAxisEntity::checkCollision(const Ray& ray, FaceSample& collisionPoint) const
{
	PR_ASSERT(isFrozen(), "has to be frozen")
	PR_GUARD_PROFILE();

	Ray local = ray;
	local.setStartPosition(invTransform() * ray.startPosition());
	local.setDirection((invDirectionMatrix() * ray.direction()).normalized());

	float t   = std::numeric_limits<float>::max();
	int found = -1;
	Eigen::Vector3f vertex;
	BoundingBox::FaceSide side;

	for (int i = 0; i < 3; ++i) {
		float tmp;
		Eigen::Vector3f tmpVertex;
		BoundingBox::FaceSide tmpSide;
		if (mAxisBoundingBox_Cache[i].intersects(local, tmpVertex, tmp, tmpSide)) {
			if (tmp > 0 && tmp < t) {
				t	  = tmp;
				found  = i;
				vertex = tmpVertex;
				side   = tmpSide;
			}
		}
	}

	if (found >= 0) {
		PR_ASSERT(found < 3, "found can't be greater than 2");

		collisionPoint.P = transform() * vertex;

		Plane plane		  = mAxisBoundingBox_Cache[found].getFace(side);
		collisionPoint.Ng = (directionMatrix() * plane.normal()).normalized();
		Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

		float u, v;
		plane.project(vertex, u, v);
		collisionPoint.UVW		= Eigen::Vector3f(u, v, 0);
		collisionPoint.Material = mMaterials[found].get();
		return true;
	}
	return false;
}

FaceSample CoordinateAxisEntity::getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const
{
	PR_ASSERT(isFrozen(), "has to be frozen")
	PR_GUARD_PROFILE();

	auto ret = sampler.generate3D(sample);

	int proj = Projection::map(ret(0), 0, 3 * 6 - 1); // Get randomly a face

	int elem = proj / 3;
	PR_ASSERT(elem >= 0 && elem < 3, "elem has to be between 0 and 2");

	BoundingBox::FaceSide side = (BoundingBox::FaceSide)(proj % 6);
	Plane plane				   = mAxisBoundingBox_Cache[elem].getFace(side);

	FaceSample fp;
	fp.P  = transform() * (plane.xAxis() * ret(1) + plane.yAxis() * ret(2));
	fp.Ng = (directionMatrix() * plane.normal()).normalized();
	Projection::tangent_frame(fp.Ng, fp.Nx, fp.Ny);
	fp.UVW		= Eigen::Vector3f(ret(1), ret(2), 0);
	fp.Material = mMaterials[elem].get();

	pdf = 1;
	return fp;
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
