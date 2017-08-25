#include "SphereEntity.h"
#include "geometry/Sphere.h"
#include "ray/Ray.h"
#include "shader/FacePoint.h"

#include "material/Material.h"
#include "math/Projection.h"

#include "performance/Performance.h"

namespace PR {
SphereEntity::SphereEntity(uint32 id, const std::string& name, float r)
	: RenderEntity(id, name)
	, mRadius(r)
	, mMaterial(nullptr)
	, mPDF_Cache(0.0f)
{
}

SphereEntity::~SphereEntity()
{
}

std::string SphereEntity::type() const
{
	return "sphere";
}

bool SphereEntity::isLight() const
{
	return mMaterial ? mMaterial->isLight() : false;
}

constexpr float P = 1.6075f;
float SphereEntity::surfaceArea(Material* m) const
{
	PR_GUARD_PROFILE();

	if (!m || m == mMaterial.get()) {
		const auto s = flags() & EF_LocalArea ? Eigen::Vector3f(1, 1, 1) : scale();

		const float a = s(0) * mRadius;
		const float b = s(1) * mRadius;
		const float c = s(2) * mRadius;

		// Knud Thomsen’s Formula
		const float t = (std::pow(a * b, P) + std::pow(a * c, P) + std::pow(b * c, P)) / 3;
		return 4 * PR_PI * std::pow(t, 1 / P);
	} else {
		return 0;
	}
}

void SphereEntity::setMaterial(const std::shared_ptr<Material>& m)
{
	mMaterial = m;
}

const std::shared_ptr<Material>& SphereEntity::material() const
{
	return mMaterial;
}

void SphereEntity::setRadius(float f)
{
	mRadius = f;
}

float SphereEntity::radius() const
{
	return mRadius;
}

bool SphereEntity::isCollidable() const
{
	return mMaterial && mMaterial->canBeShaded() && mRadius >= PR_EPSILON;
}

float SphereEntity::collisionCost() const
{
	return 1;
}

BoundingBox SphereEntity::localBoundingBox() const
{
	return BoundingBox(Eigen::Vector3f(mRadius, mRadius, mRadius),
					   Eigen::Vector3f(-mRadius, -mRadius, -mRadius));
}

RenderEntity::Collision SphereEntity::checkCollision(const Ray& ray) const
{
	PR_GUARD_PROFILE();

	Ray local = ray;
	local.setOrigin(invTransform() * ray.origin());
	local.setDirection((invDirectionMatrix() * ray.direction()).normalized());

	Sphere sphere(Eigen::Vector3f(0, 0, 0), mRadius);
	RenderEntity::Collision c;
	Sphere::Intersection in = sphere.intersects(local);
	c.Successful			= in.Successful;

	if (!in.Successful)
		return c;

	c.Point.P = transform() * in.Position;

	c.Point.Ng = (directionMatrix() * in.Position).normalized();
	Projection::tangent_frame(c.Point.Ng, c.Point.Nx, c.Point.Ny);

	const Eigen::Vector2f uv = Projection::sphereUV(c.Point.Ng);
	c.Point.UVW				 = Eigen::Vector3f(uv(0), uv(1), 0);
	c.Point.Material		 = material().get();

	return c;
}

RenderEntity::FacePointSample SphereEntity::sampleFacePoint(const Eigen::Vector3f& rnd) const
{
	PR_GUARD_PROFILE();

	RenderEntity::FacePointSample sm;

	Eigen::Vector3f n = Projection::sphere_coord(rnd(0) * 2 * PR_PI, rnd(1) * PR_PI);
	sm.PDF_A		  = mPDF_Cache;

	sm.Point.Ng = (directionMatrix() * n).normalized();
	Projection::tangent_frame(sm.Point.Ng, sm.Point.Nx, sm.Point.Ny);

	sm.Point.P				 = transform() * (n * mRadius);
	const Eigen::Vector2f uv = Projection::sphereUV(sm.Point.Ng);
	sm.Point.UVW			 = Eigen::Vector3f(uv(0), uv(1), 0);
	sm.Point.Material		 = material().get();

	return sm;
}

void SphereEntity::setup(RenderContext* context)
{
	RenderEntity::setup(context);

	if (mMaterial)
		mMaterial->setup(context);

	const float area = surfaceArea(nullptr);
	mPDF_Cache		 = (area > PR_EPSILON ? 1.0f / area : 0);
}
}
