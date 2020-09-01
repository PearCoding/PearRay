#include "3d/OpenGLHeaders.h"

#include "3d/shader/ColorShader.h"
#include "GridEntity.h"

namespace PR {
namespace UI {
GridEntity::GridEntity(int count, float wx, float wy)
	: GraphicEntity()
	, mGridSize(std::make_pair(wx, wy))
	, mGridCount(std::max(1, count))
{
	setupBuffer();

	std::shared_ptr<ColorShader> shader = std::make_shared<ColorShader>();
	shader->setColor(Vector4f(0.5f, 0.5f, 0.5f, 1));

	setShader(shader);
}

GridEntity::~GridEntity()
{
}

void GridEntity::setGridSize(float wx, float wy)
{
	mGridSize = std::make_pair(wx, wy);
	setupBuffer();
}

void GridEntity::setGridCount(int count)
{
	mGridCount = std::max(1, count);
	setupBuffer();
}

void GridEntity::setupBuffer()
{
	std::vector<float> vertices;

	auto append = [&](const Vector3f& p0, const Vector3f& p1) {
		vertices.push_back(p0[0]);
		vertices.push_back(p0[1]);
		vertices.push_back(p0[2]);
		vertices.push_back(p1[0]);
		vertices.push_back(p1[1]);
		vertices.push_back(p1[2]);
	};

	float stepX = mGridSize.first / mGridCount;
	float stepY = mGridSize.second / mGridCount;

	// X
	for (int i = 0; i < mGridCount + 1; ++i)
		append(Vector3f(-mGridSize.first / 2 + i * stepX, -mGridSize.second / 2, 0),
			   Vector3f(-mGridSize.first / 2 + i * stepX, mGridSize.second / 2, 0));

	// Y
	for (int i = 0; i < mGridCount + 1; ++i)
		append(Vector3f(-mGridSize.first / 2, -mGridSize.second / 2 + i * stepY, 0),
			   Vector3f(mGridSize.first / 2, -mGridSize.second / 2 + i * stepY, 0));

	setVertices(vertices);
	setDrawMode(GL_LINES);
}
} // namespace UI
} // namespace PR