#include "BoxEntity.h"
#include "3d/shader/ColorShader.h"

namespace PR {
namespace UI {
BoxEntity::BoxEntity(const BoundingBox& aabb)
	: GraphicEntity()
	, mAABB(aabb)
{
	setupBuffer();
	setShader(std::make_shared<ColorShader>(Vector4f(0.75f, 0.75f, 0.75f, 1)));
}

BoxEntity::~BoxEntity()
{
}

void BoxEntity::setAABB(const BoundingBox& o)
{
	mAABB = o;
	setupBuffer();
}

void BoxEntity::setupBuffer()
{
	std::vector<float> vertices;
	std::vector<uint32> indices;

	auto append = [&](const Vector3f& p0) {
		vertices.push_back(p0[0]);
		vertices.push_back(p0[1]);
		vertices.push_back(p0[2]);
	};
	for (int i = 0; i < 8; ++i)
		append(mAABB.corner(i));

	BoundingBox::triangulateIndices({ 0, 1, 2, 3, 4, 5, 6, 7 }, indices);

	setVertices(vertices);
	setIndices(indices);
}

} // namespace UI
} // namespace PR