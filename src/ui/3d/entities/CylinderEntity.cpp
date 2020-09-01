#include "CylinderEntity.h"
#include "3d/shader/ColorShader.h"
#include "geometry/Cylinder.h"

namespace PR {
namespace UI {
CylinderEntity::CylinderEntity(float height, float topRadius, float bottomRadius)
	: GraphicEntity()
	, mHeight(height)
	, mTopRadius(topRadius)
	, mBottomRadius(bottomRadius)
	, mSectionCount(16)
{
	setupGeometry();

	std::shared_ptr<ColorShader> shader = std::make_shared<ColorShader>();
	shader->setColor(Vector4f(0.5f, 0.5f, 0.5f, 1));

	setShader(shader);
}

CylinderEntity::~CylinderEntity()
{
}

void CylinderEntity::setHeight(float h)
{
	mHeight = h;
	setupGeometry();
}

void CylinderEntity::setTopRadius(float r)
{
	mTopRadius = r;
	setupGeometry();
}

void CylinderEntity::setBottomRadius(float r)
{
	mBottomRadius = r;
	setupGeometry();
}

void CylinderEntity::setSectionCount(unsigned int c)
{
	mSectionCount = c;
	setupGeometry();
}

void CylinderEntity::setupGeometry()
{
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	Cylinder::triangulate(Vector3f(0, 0, 0), Vector3f(0, 0, mHeight), mTopRadius, mBottomRadius, mSectionCount, vertices);
	Cylinder::triangulateIndices(0, 1, mSectionCount, indices);

	setVertices(vertices);
	setIndices(indices);
}
} // namespace UI
} // namespace PR
