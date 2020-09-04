#include "3d/OpenGLHeaders.h"

#include "3d/shader/ColorShader.h"
#include "PointMapEntity.h"
#include "math/Spherical.h"
#include "math/Triangulation.h"

namespace PR {
namespace UI {
PointMapEntity::PointMapEntity(PointMapEntity::MapType type)
	: GraphicEntity()
	, mMapType(type)
	, mPopulateColor(true)
{
	setTwoSided(true);
	setShader(std::make_shared<ColorShader>());
	setDrawMode(GL_TRIANGLES);
}

PointMapEntity::~PointMapEntity()
{
}

template <typename T>
static inline T lerp(const T& a, const T& b, float t)
{
	return a * (1 - t) + b * t;
}

// t in [0,1]
static inline Vector3f colormap(float t)
{
	if (t > 0.5f)
		return lerp(Vector3f(0, 1, 0), Vector3f(1, 0, 0), (t - 0.5f) * 2);
	else
		return lerp(Vector3f(0, 0, 1), Vector3f(0, 1, 0), t * 2);
}

void PointMapEntity::build(const std::vector<Vector2f>& points, const std::vector<float>& values)
{
	PR_ASSERT(points.size() == values.size(), "Expected points and values to be of same size");

	// We need at least three points for a soup of triangles
	if (points.size() < 3)
		return;

	//////////////////// Vertices
	std::vector<float> vertices;
	vertices.reserve(points.size() * 3);
	auto add = [&](const Vector3f& v) {vertices.push_back(v[0]); vertices.push_back(v[1]); vertices.push_back(v[2]); };

	std::vector<Triangulation::Triangle> triangles;
	switch (mMapType) {
	case MapType::MT_Z:
		for (size_t i = 0; i < points.size(); ++i)
			add(Vector3f(points[i](0), points[i](1), values[i]));
		triangles = Triangulation::triangulate2D(points);
		break;
	case MapType::MT_Spherical: {
		for (size_t i = 0; i < points.size(); ++i) {
			const Vector3f D = Spherical::cartesian(points[i](0), points[i](1));
			add(values[i] * D);
		}
		std::vector<Vector3f> vpts; // TODO: Get rid of this temporary vector
		vpts.reserve(points.size());
		for (size_t i = 0; i < points.size(); ++i)
			vpts.push_back(Vector3f(vertices[i * 3 + 0], vertices[i * 3 + 1], vertices[i * 3 + 2]));
		triangles = Triangulation::triangulate3D(vpts);
	} break;
	}

	//////////////////// Indices
	if (triangles.empty()) // Backoff if no triangles could be generated (less than 3 points)
		return;

	std::vector<uint32> indices;
	indices.reserve(triangles.size() * 3);
	for (const auto& triangle : triangles) {
		indices.push_back(triangle.V0);
		indices.push_back(triangle.V1);
		indices.push_back(triangle.V2);
	}

	//////////////////// Color
	if (mPopulateColor) {
		std::vector<float> colors;
		colors.reserve(vertices.size());
		for (size_t i = 0; i < values.size(); ++i) {
			const Vector3f c = colormap(values[i]);
			for (int j = 0; j < 3; ++j)
				colors.push_back(c(j));
		}
		commitColors(std::move(colors));
	}

	//////////////////// Commit
	commitVertices(std::move(vertices));
	commitIndices(std::move(indices));

	requestRebuild();
}
} // namespace UI
} // namespace PR