#include "SphereEntity.h"
#include "3d/shader/ColorShader.h"
#include "geometry/Sphere.h"

namespace PR {
namespace UI {
SphereEntity::SphereEntity(float radius)
	: GraphicEntity()
	, mRadius(radius)
	, mStackCount(16)
	, mSliceCount(32)
{
	setupGeometry();

	std::shared_ptr<ColorShader> shader = std::make_shared<ColorShader>();
	shader->setColor(Vector4f(0.5f, 0.5f, 0.5f, 1));

	setShader(shader);
}

SphereEntity::~SphereEntity()
{
}

void SphereEntity::setRadius(float r)
{
	mRadius = r;
	setupGeometry();
}

void SphereEntity::setStackCount(unsigned int c)
{
	mStackCount = c;
	setupGeometry();
}

void SphereEntity::setSliceCount(unsigned int c)
{
	mSliceCount = c;
	setupGeometry();
}

void SphereEntity::setupGeometry()
{
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	Sphere::triangulate(Vector3f(0, 0, 0), mRadius, mStackCount, mSliceCount, vertices);
	Sphere::triangulateIndices(mStackCount, mSliceCount, indices);

	setVertices(vertices);
	setIndices(indices);
}
} // namespace UI
} // namespace PR
