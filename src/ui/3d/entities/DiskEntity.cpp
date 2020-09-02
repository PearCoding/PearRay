#include "DiskEntity.h"
#include "3d/shader/ColorShader.h"
#include "geometry/Disk.h"

namespace PR {
namespace UI {
DiskEntity::DiskEntity(float radius)
	: GraphicEntity()
	, mRadius(radius)
	, mSectionCount(32)
{
	setTwoSided(true); // Default is true
	setupGeometry();
	setShader(std::make_shared<ColorShader>(Vector4f(0.75f, 0.75f, 0.75f, 1)));
}

DiskEntity::~DiskEntity()
{
}

void DiskEntity::setRadius(float r)
{
	mRadius = r;
	setupGeometry();
}

void DiskEntity::setSectionCount(uint32 c)
{
	mSectionCount = c;
	setupGeometry();
}

Vector3f DiskEntity::normal() const
{
	return normalTransform() * Vector3f(0, 0, 1);
}

void DiskEntity::setupGeometry()
{
	std::vector<float> vertices;
	std::vector<uint32> indices;

	Disk::triangulate(Vector3f(0, 0, 0), mRadius, mSectionCount, vertices);
	Disk::triangulateIndices(0, mSectionCount, indices);

	setVertices(vertices);
	setIndices(indices);
}
} // namespace UI
}