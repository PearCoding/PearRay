#include "MeshEntity.h"

#include "3d/shader/GoochShader.h"

namespace PR {
namespace UI {
MeshEntity::MeshEntity()
	: GraphicEntity()
{
	std::shared_ptr<GoochShader> shader = std::make_shared<GoochShader>();
	shader->setColorWarm(Vector4f(1, 0.8f, 0.8f, 1));
	shader->setColorCold(Vector4f(0.5f, 0.2f, 0, 1));

	setShader(shader);
}

MeshEntity::~MeshEntity()
{
}

void MeshEntity::setMesh(const std::vector<float>& vertices, const std::vector<uint32>& indices, const std::vector<float>& normals)
{
	setVertices(vertices);
	setIndices(indices);
	setNormals(normals);
}
} // namespace UI
} // namespace PR