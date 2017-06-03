#include "SphereEntity.h"
#include "geometry/Sphere.h"
#include "ray/Ray.h"
#include "shader/FaceSample.h"

#include "material/Material.h"
#include "math/Projection.h"
#include "sampler/Sampler.h"

#include "performance/Performance.h"

namespace PR {
SphereEntity::SphereEntity(uint32 id, const std::string& name, float r)
	: RenderEntity(id, name)
	, mRadius(r)
	, mMaterial(nullptr)
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

bool SphereEntity::checkCollision(const Ray& ray, FaceSample& collisionPoint) const
{
	PR_GUARD_PROFILE();

	Ray local = ray;
	local.setOrigin(invTransform() * ray.origin());
	local.setDirection((invDirectionMatrix() * ray.direction()).normalized());

	Sphere sphere(Eigen::Vector3f(0, 0, 0), mRadius);
	float t;
	Eigen::Vector3f collisionPos;
	if (!sphere.intersects(local, collisionPos, t))
		return false;

	collisionPoint.P = transform() * collisionPos;

	collisionPoint.Ng = (directionMatrix() * collisionPos).normalized();
	Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

	const Eigen::Vector2f uv = Projection::sphereUV(collisionPoint.Ng);
	collisionPoint.UVW		 = Eigen::Vector3f(uv(0), uv(1), 0);

	collisionPoint.Material = material().get();

	return true;
}

FaceSample SphereEntity::getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const
{
	PR_GUARD_PROFILE();

	FaceSample p;

	Eigen::Vector2f s = sampler.generate2D(sample);
	Eigen::Vector3f n = Projection::sphere_coord(s(0) * 2 * PR_PI, s(1) * PR_PI);
	pdf				  = 1;

	p.Ng = (directionMatrix() * n).normalized();
	Projection::tangent_frame(p.Ng, p.Nx, p.Ny);

	p.P						 = transform() * (n * mRadius);
	const Eigen::Vector2f uv = Projection::sphereUV(p.Ng);
	p.UVW					 = Eigen::Vector3f(uv(0), uv(1), 0);
	p.Material				 = material().get();

	return p;
}

void SphereEntity::setup(RenderContext* context)
{
	RenderEntity::setup(context);

	if (mMaterial)
		mMaterial->setup(context);
}
}
