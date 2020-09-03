#include "3d/OpenGLHeaders.h"

#include "3d/shader/ColorShader.h"
#include "PointMapEntity.h"
#include "math/Delaunay.h"
#include "math/Spherical.h"

namespace PR {
namespace UI {
PointMapEntity::PointMapEntity(PointMapEntity::MapType type)
	: GraphicEntity()
	, mMapType(type)
{
	setTwoSided(true);
	setShader(std::make_shared<ColorShader>());
	setDrawMode(GL_TRIANGLES);
}

PointMapEntity::~PointMapEntity()
{
}

void PointMapEntity::build(const std::vector<Vector2f>& points, const std::vector<float>& values)
{
	//////////////////// Vertices
	std::vector<float> vertices;
	vertices.reserve(points.size() * 3);
	auto add = [&](const Vector3f& v) {vertices.push_back(v[0]); vertices.push_back(v[1]); vertices.push_back(v[2]); };

	switch (mMapType) {
	case MapType::MT_Z:
		for (size_t i = 0; i < points.size(); ++i) {
			add(Vector3f(points[i](0), points[i](1), values[i]));
		}
		break;
	case MapType::MT_Spherical:
		for (size_t i = 0; i < points.size(); ++i) {
			const Vector3f D = Spherical::cartesian(points[i](0), points[i](1));
			add(values[i] * D);
		}
		break;
	}

	//////////////////// Indices
	auto triangles = Delaunay::triangulate2D(points);
	std::vector<uint32> indices;
	indices.reserve(triangles.size() * 3);
	for (const auto& triangle : triangles) {
		indices.push_back(triangle.V0);
		indices.push_back(triangle.V1);
		indices.push_back(triangle.V2);
	}

	//////////////////// Commit
	setVertices(vertices);
	setIndices(indices);

	requestRebuild();
}
} // namespace UI
} // namespace PR