#include "PlaneEntity.h"

#include "material/Material.h"
#include "ray/Ray.h"
#include "shader/FacePoint.h"

#include "Logger.h"
#include "math/Projection.h"

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

std::shared_ptr<Material> PlaneEntity::material() const
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
RenderEntity::FacePointSample PlaneEntity::sampleFacePoint(const Eigen::Vector3f& rnd) const
{
	RenderEntity::FacePointSample sm;
	sm.Point.P = mGlobalPlane_Cache.position()
				 + mGlobalPlane_Cache.xAxis() * rnd(0)
				 + mGlobalPlane_Cache.yAxis() * rnd(1);

	sm.Point.Ng = mGlobalPlane_Cache.normal();
	sm.Point.Nx = mXAxisN_Cache;
	sm.Point.Ny = mYAxisN_Cache;

	sm.Point.UVW	  = Eigen::Vector3f(rnd(0), rnd(1), 0);
	sm.Point.Material = material().get();
	sm.PDF_A		  = mPDF_Cache;

	return sm;
}

void PlaneEntity::onFreeze(RenderContext* context)
{
	RenderEntity::onFreeze(context);

	mGlobalPlane_Cache.setPosition(transform() * mPlane.position());

	const Eigen::Vector3f px = transform() * (mPlane.position() + mPlane.xAxis());
	const Eigen::Vector3f py = transform() * (mPlane.position() + mPlane.yAxis());

	mGlobalPlane_Cache.setAxis(px - mGlobalPlane_Cache.position(),
							   py - mGlobalPlane_Cache.position());

	mXAxisN_Cache = mGlobalPlane_Cache.xAxis().normalized();
	mYAxisN_Cache = mGlobalPlane_Cache.yAxis().normalized();

	mPDF_Cache = 1.0f / mGlobalPlane_Cache.surfaceArea();

	PR_LOG(L_INFO) << "Plane: px[" << px << "] py[" << py << "] A " << mGlobalPlane_Cache.surfaceArea() << std::endl;
	PR_LOG(L_INFO) << "Plane: axisX[" << mGlobalPlane_Cache.xAxis() << "] yAxis[" << mGlobalPlane_Cache.yAxis() << "]" << std::endl;

	// Check up
	if (std::abs((mGlobalPlane_Cache.normal()).squaredNorm() - 1) > PR_EPSILON)
		PR_LOG(L_WARNING) << "Plane entity " << name() << " has non unit normal vector!" << std::endl;

	if ((mGlobalPlane_Cache.xAxis()).squaredNorm() <= PR_EPSILON)
		PR_LOG(L_WARNING) << "Plane entity " << name() << " has zero x axis!" << std::endl;

	if ((mGlobalPlane_Cache.yAxis()).squaredNorm() <= PR_EPSILON)
		PR_LOG(L_WARNING) << "Plane entity " << name() << " has zero y axis!" << std::endl;

	if (mGlobalPlane_Cache.surfaceArea() <= PR_EPSILON)
		PR_LOG(L_WARNING) << "Plane entity " << name() << " has zero enclosed area!" << std::endl;

	if (mMaterial)
		mMaterial->freeze(context);
}
} // namespace PR
