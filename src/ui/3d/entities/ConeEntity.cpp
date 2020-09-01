#include "ConeEntity.h"
#include "3d/shader/ColorShader.h"
#include "geometry/Cone.h"

namespace PR {
namespace UI {
ConeEntity::ConeEntity(float height, float radius)
	: GraphicEntity()
	, mHeight(height)
	, mRadius(radius)
	, mSectionCount(16)
{
	setupGeometry();

	std::shared_ptr<ColorShader> shader = std::make_shared<ColorShader>();
	shader->setColor(Vector4f(0.5f, 0.5f, 0.5f, 1));

	setShader(shader);
}

ConeEntity::~ConeEntity()
{
}

void ConeEntity::setHeight(float h)
{
	mHeight = h;
	setupGeometry();
}

void ConeEntity::setRadius(float r)
{
	mRadius = r;
	setupGeometry();
}

void ConeEntity::setSectionCount(unsigned int c)
{
	mSectionCount = c;
	setupGeometry();
}

void ConeEntity::setupGeometry()
{
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	Cone::triangulate(Vector3f(0, 0, 0), Vector3f(0, 0, mHeight), mRadius, mSectionCount, vertices);
	Cone::triangulateIndices(0, 1, mSectionCount, indices);

	setVertices(vertices);
	setIndices(indices);
}
} // namespace UI
} // namespace PR