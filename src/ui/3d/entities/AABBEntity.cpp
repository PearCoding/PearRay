#include "3d/OpenGLHeaders.h"

#include "3d/shader/ColorShader.h"
#include "AABBEntity.h"

namespace PR {
namespace UI {
AABBEntity::AABBEntity(const BoundingBox& aabb)
	: GraphicEntity()
	, mAABB(aabb)
{
	setupBuffer();

	std::shared_ptr<ColorShader> shader = std::make_shared<ColorShader>();
	shader->setColor(Vector4f(1, 0, 0, 1));

	setShader(shader);
}

AABBEntity::~AABBEntity()
{
}

void AABBEntity::setAABB(const BoundingBox& o)
{
	mAABB = o;
	setupBuffer();
}

void AABBEntity::setupBuffer()
{
	std::vector<float> vertices;

	// All 8 corners
	Vector3f LBF = mAABB.corner(0);
	Vector3f RBF = mAABB.corner(1);
	Vector3f LTF = mAABB.corner(2);
	Vector3f RTF = mAABB.corner(3);
	Vector3f LBB = mAABB.corner(4);
	Vector3f RBB = mAABB.corner(5);
	Vector3f LTB = mAABB.corner(6);
	Vector3f RTB = mAABB.corner(7);

	auto append = [&](const Vector3f& p0, const Vector3f& p1) {
		vertices.push_back(p0[0]);
		vertices.push_back(p0[1]);
		vertices.push_back(p0[2]);
		vertices.push_back(p1[0]);
		vertices.push_back(p1[1]);
		vertices.push_back(p1[2]);
	};

	// All 12 edges
	append(LBF, RBF);
	append(LTF, RTF);
	append(LBB, RBB);
	append(LTB, RTB);

	append(LBF, LTF);
	append(RBF, RTF);
	append(LBB, LTB);
	append(RBB, RTB);

	append(LBF, LBB);
	append(RBF, RBB);
	append(LTF, LTB);
	append(RTF, RTB);

	setVertices(vertices);
	setDrawMode(GL_LINES);
}
} // namespace UI
} // namespace PR