#include "PointLight.h"
#include "ray/Ray.h"
#include "material/Material.h"
#include "geometry/FacePoint.h"
#include "geometry/Sphere.h"

namespace PR
{
	const float BOUNDARY_WIDTH = 1.e-5f;
	PointLight::PointLight(const std::string& name, Entity* parent) :
		RenderEntity(name, parent)
	{
	}

	PointLight::~PointLight()
	{
	}

	std::string PointLight::type() const
	{
		return "pointLight";
	}

	bool PointLight::isLight() const
	{
		return mMaterial ? mMaterial->isLight() : false;
	}

	uint32 PointLight::maxLightSamples() const
	{
		return 1;
	}

	void PointLight::setMaterial(Material* m)
	{
		mMaterial = m;
	}

	Material* PointLight::material() const
	{
		return mMaterial;
	}

	bool PointLight::isCollidable() const
	{
		return true;
	}

	BoundingBox PointLight::localBoundingBox() const
	{
		return BoundingBox(PM::pm_Set(BOUNDARY_WIDTH, BOUNDARY_WIDTH, BOUNDARY_WIDTH, 1),
			PM::pm_Set(-BOUNDARY_WIDTH, -BOUNDARY_WIDTH, -BOUNDARY_WIDTH, 1));
	}

	bool PointLight::checkCollision(const Ray& ray, FacePoint& collisionPoint)
	{
		PM::vec3 v;
		//bool found = Sphere(position(), BOUNDARY_WIDTH).intersects(ray, v);
		bool found = worldBoundingBox().intersects(ray, v);
		collisionPoint.setVertex(v);
		return found;
	}

	void PointLight::apply(Ray& in, const FacePoint& point, Renderer* renderer)
	{
		if (mMaterial)
		{
			mMaterial->apply(in, this, point, renderer);
		}
	}

	FacePoint PointLight::getRandomFacePoint(Sampler& sampler, Random& random) const
	{
		FacePoint p;
		p.setVertex(position());
		return p;
	}
}