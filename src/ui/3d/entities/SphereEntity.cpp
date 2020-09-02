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
	setShader(std::make_shared<ColorShader>(Vector4f(0.5f, 0.5f, 0.5f, 1)));
}

SphereEntity::~SphereEntity()
{
}

void SphereEntity::setRadius(float r)
{
	mRadius = r;
	setupGeometry();
}

void SphereEntity::setStackCount(uint32 c)
{
	mStackCount = c;
	setupGeometry();
}

void SphereEntity::setSliceCount(uint32 c)
{
	mSliceCount = c;
	setupGeometry();
}

void SphereEntity::setupGeometry()
{
	std::vector<float> vertices;
	std::vector<uint32> indices;

	Sphere::triangulate(Vector3f(0, 0, 0), mRadius, mStackCount, mSliceCount, vertices);
	Sphere::triangulateIndices(mStackCount, mSliceCount, indices);

	setVertices(vertices);
	setIndices(indices);
}
} // namespace UI
} // namespace PR
