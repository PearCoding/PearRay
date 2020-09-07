#include "3d/OpenGLHeaders.h"

#include "3d/shader/ColorShader.h"
#include "GridMapEntity.h"
#include "math/Spherical.h"

namespace PR {
namespace UI {
GridMapEntity::GridMapEntity(MapType type)
	: GraphicEntity()
	, mMapType(type)
	, mPopulateColor(true)
{
	setTwoSided(true);
	setShader(std::make_shared<ColorShader>());
	setDrawMode(GL_TRIANGLES);
}

GridMapEntity::~GridMapEntity()
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

void GridMapEntity::build(size_t slice, const std::vector<float>& values)
{
	// We need at least three points for a soup of triangles
	if (values.size() < 3)
		return;

	//////////////////// Vertices
	std::vector<float> vertices;
	vertices.reserve(values.size() * 3);
	auto add = [&](const Vector3f& v) {vertices.push_back(v[0]); vertices.push_back(v[1]); vertices.push_back(v[2]); };

	size_t sx = slice;
	size_t sy = values.size() / slice;

	switch (mMapType) {
	case MapType::MT_Z:
		for (size_t j = 0; j < sy; ++j)
			for (size_t i = 0; i < sx; ++i)
				add(Vector3f(2 * i / float(sx - 1) - 1, 2 * j / float(sx - 1) - 1, values[j * sx + i]));
		break;
	case MapType::MT_Spherical: {
		for (size_t j = 0; j < sy; ++j) {
			for (size_t i = 0; i < sx; ++i) {
				const float theta = 0.5f * PR_PI * j / float(sy - 1);
				const float phi	  = 2 * PR_PI * i / float(sx - 1);
				const Vector3f D  = Spherical::cartesian(theta, phi);
				add(values[j * sx + i] * D);
			}
		}
	} break;
	}

	//////////////////// Indices
	std::vector<uint32> indices;
	indices.reserve((sy * sx) * 3 * 2);
	for (size_t j = 0; j < sy; ++j) {
		for (size_t i = 0; i < sx; ++i) {
			uint32 row1 = j * sx;
			uint32 row2 = (j + 1) * sx;
			uint32 ni	= (i + 1) % sx;

			// Triangle 1
			indices.push_back(row1 + i);
			indices.push_back(row1 + ni);
			indices.push_back(row2 + ni);

			// Triangle 2
			indices.push_back(row1 + i);
			indices.push_back(row2 + ni);
			indices.push_back(row2 + i);
		}
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