#include "PlaneEntity.h"

#include "material/Material.h"
#include "ray/Ray.h"
#include "shader/FacePoint.h"

#include "Logger.h"
#include "math/Projection.h"
#include "sampler/Sampler.h"

#include "performance/Performance.h"

namespace PR {
PlaneEntity::PlaneEntity(uint32 id, const std::string& name, const Plane& plane)
	: RenderEntity(id, name)
	, mPlane(plane)
	, mMaterial(nullptr)
{
}

PlaneEntity::~PlaneEntity()
{
}

std::string PlaneEntity::type() const
{
	return "plane";
}

bool PlaneEntity::isLight() const
{
	return mMaterial ? mMaterial->isLight() : false;
}

float PlaneEntity::surfaceArea(Material* m) const
{
	if (!m || m == mMaterial.get()) {
		if ((flags() & EF_LocalArea) == 0) {
			if (isFrozen())
				return mGlobalPlane_Cache.surfaceArea();
			else
				return (directionMatrix() * mPlane.xAxis()).norm() * (directionMatrix() * mPlane.yAxis()).norm();
		} else {
			return mPlane.surfaceArea();
		}
	} else {
		return 0;
	}
}

void PlaneEntity::setMaterial(const std::shared_ptr<Material>& m)
{
	mMaterial = m;
}

const std::shared_ptr<Material>& PlaneEntity::material() const
{
	return mMaterial;
}

void PlaneEntity::setPlane(const Plane& plane)
{
	mPlane = plane;
}

const Plane& PlaneEntity::plane() const
{
	return mPlane;
}

bool PlaneEntity::isCollidable() const
{
	return mMaterial && mMaterial->canBeShaded();
}

float PlaneEntity::collisionCost() const
{
	return 2;
}

BoundingBox PlaneEntity::localBoundingBox() const
{
	return mPlane.toBoundingBox();
}

RenderEntity::Collision PlaneEntity::checkCollision(const Ray& ray) const
{
	PR_GUARD_PROFILE();

	RenderEntity::Collision c;
	Plane::Intersection in = mGlobalPlane_Cache.intersects(ray);
	c.Successful		   = in.Successful;

	if (c.Successful) {
		c.Point.P = in.Position;

		c.Point.Ng = mGlobalPlane_Cache.normal();
		c.Point.Nx = mXAxisN_Cache;
		c.Point.Ny = mYAxisN_Cache;

		c.Point.UVW		 = Eigen::Vector3f(in.UV(0), in.UV(1), 0);
		c.Point.Material = material().get();

		return c;
	}

	return c;
}

// World space
RenderEntity::FacePointSample PlaneEntity::sampleFacePoint(Sampler& sampler, uint32 sample) const
{
	auto s = sampler.generate2D(sample);

	RenderEntity::FacePointSample sm;
	sm.Point.P = mGlobalPlane_Cache.position()
				 + mGlobalPlane_Cache.xAxis() * s(0)
				 + mGlobalPlane_Cache.yAxis() * s(1);

	sm.Point.Ng = mGlobalPlane_Cache.normal();
	sm.Point.Nx = mXAxisN_Cache;
	sm.Point.Ny = mYAxisN_Cache;

	sm.Point.UVW	  = Eigen::Vector3f(s(0), s(1), 0);
	sm.Point.Material = material().get();
	sm.PDF			  = 1;

	return sm;
}

void PlaneEntity::onFreeze()
{
	RenderEntity::onFreeze();

	mGlobalPlane_Cache.setPosition(transform() * mPlane.position());

	const Eigen::Vector3f px = transform() * (mPlane.position() + mPlane.xAxis());
	const Eigen::Vector3f py = transform() * (mPlane.position() + mPlane.yAxis());

	mGlobalPlane_Cache.setAxis(px - mGlobalPlane_Cache.position(),
							   py - mGlobalPlane_Cache.position());

	mXAxisN_Cache = mGlobalPlane_Cache.xAxis().normalized();
	mYAxisN_Cache = mGlobalPlane_Cache.yAxis().normalized();

	// Check up
	if (std::abs((mGlobalPlane_Cache.normal()).squaredNorm() - 1) > PR_EPSILON)
		PR_LOGGER.logf(L_Warning, M_Entity, "Plane entity %s has a non unit normal vector!", name().c_str());

	if ((mGlobalPlane_Cache.xAxis()).squaredNorm() <= PR_EPSILON)
		PR_LOGGER.logf(L_Warning, M_Entity, "Plane entity %s has zero x axis!", name().c_str());

	if ((mGlobalPlane_Cache.yAxis()).squaredNorm() <= PR_EPSILON)
		PR_LOGGER.logf(L_Warning, M_Entity, "Plane entity %s has zero y axis!", name().c_str());

	if (mGlobalPlane_Cache.surfaceArea() <= PR_EPSILON)
		PR_LOGGER.logf(L_Warning, M_Entity, "Plane entity %s has zero enclosed area!", name().c_str());
}

void PlaneEntity::setup(RenderContext* context)
{
	RenderEntity::setup(context);

	if (mMaterial)
		mMaterial->setup(context);
}
}
