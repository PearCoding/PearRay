#include "3d/OpenGLHeaders.h"

#include "3d/shader/ColorShader.h"
#include "Axis3DEntity.h"

namespace PR {
namespace UI {
Axis3DEntity::Axis3DEntity(float edgeLength)
	: GraphicEntity()
	, mEdgeLength(edgeLength)
{
	setupBuffer();

	std::shared_ptr<ColorShader> shader = std::make_shared<ColorShader>();
	setShader(shader);
}

Axis3DEntity::~Axis3DEntity()
{
}

void Axis3DEntity::setEdgeLength(float f)
{
	mEdgeLength = f;
	setupBuffer();
}

void Axis3DEntity::setupBuffer()
{
	std::vector<float> vertices;
	std::vector<float> colors;

	auto append = [&](const Vector3f& p0, const Vector3f& p1, const Vector3f& c0, const Vector3f& c1) {
		vertices.push_back(p0[0]);
		vertices.push_back(p0[1]);
		vertices.push_back(p0[2]);
		vertices.push_back(p1[0]);
		vertices.push_back(p1[1]);
		vertices.push_back(p1[2]);

		colors.push_back(c0(0));
		colors.push_back(c0(1));
		colors.push_back(c0(2));
		colors.push_back(c1(0));
		colors.push_back(c1(1));
		colors.push_back(c1(2));
	};

	append(Vector3f(0, 0, 0), mEdgeLength * Vector3f(1, 0, 0), Vector3f(1, 0, 0), Vector3f(1, 0, 0));
	append(Vector3f(0, 0, 0), mEdgeLength * Vector3f(0, 1, 0), Vector3f(0, 1, 0), Vector3f(0, 1, 0));
	append(Vector3f(0, 0, 0), mEdgeLength * Vector3f(0, 0, 1), Vector3f(0, 0, 1), Vector3f(0, 0, 1));

	setVertices(vertices);
	setColors(colors);
	setDrawMode(GL_LINES);
}
} // namespace UI
}