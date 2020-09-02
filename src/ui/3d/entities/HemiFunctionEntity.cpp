#include "3d/OpenGLHeaders.h"

#include "3d/shader/ColorShader.h"
#include "HemiFunctionEntity.h"

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

namespace PR {
namespace UI {
HemiFunctionEntity::HemiFunctionEntity(const Function& function, int nradius, int nphi)
	: GraphicEntity()
	, mFunction(function)
	, mNRadius(nradius)
	, mNPhi(nphi)
	, mNormalize(true)
{
	setupBuffer();
	setTwoSided(true);
	setShader(std::make_shared<ColorShader>());
	setDrawMode(GL_TRIANGLES);
}

HemiFunctionEntity::~HemiFunctionEntity()
{
}

void HemiFunctionEntity::setRadiusCount(int nradius)
{
	mNRadius = nradius;
}

void HemiFunctionEntity::setPhiCount(int nphi)
{
	mNPhi = nphi;
}

void HemiFunctionEntity::setFunction(const Function& f)
{
	mFunction = f;
}

void HemiFunctionEntity::enableNormalization(bool b)
{
	mNormalize = b;
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

void HemiFunctionEntity::setupBuffer()
{
	//////////////// Vertices
	std::vector<float> vertices;
	vertices.resize((mNRadius + 1) * mNPhi * 3);

	tbb::parallel_for(
		tbb::blocked_range<int>(0, mNRadius + 1),
		[&](const tbb::blocked_range<int>& r) {
			const int sy = r.begin();
			const int ey = r.end();
			for (int j = sy; j < ey; ++j) {
				for (int i = 0; i < mNPhi; ++i) {
					float r	  = std::sqrt(j / (float)mNRadius);
					float phi = 2 * PR::PR_PI * (i / (float)mNPhi);

					float x = r * std::cos(phi);
					float y = r * std::sin(phi);

					const Vector3f D = Vector3f(x, y, std::sqrt(1 - r * r));
					float val		 = mFunction(D);
					if (!std::isfinite(val))
						val = 0;

					int ind				  = j * mNPhi + i;
					vertices[ind * 3 + 0] = val * D(0);
					vertices[ind * 3 + 1] = val * D(1);
					vertices[ind * 3 + 2] = val * D(2);
				}
			}
		});

	//////////////// Extract maximum value
	float maxValue = 0;
	for (int j = 0; j <= mNRadius; ++j) {
		for (int i = 0; i < mNPhi; ++i) {
			int ind	   = j * mNPhi + i;
			Vector3f v = Vector3f(vertices[ind * 3], vertices[ind * 3 + 1], vertices[ind * 3 + 2]);
			maxValue   = std::max(maxValue, v.norm());
		}
	}

	//////////////// Color
	std::vector<float> colors;

	const auto appendC = [&](const Vector3f& p0) {
		colors.push_back(p0[0]);
		colors.push_back(p0[1]);
		colors.push_back(p0[2]);
	};

	colors.reserve(vertices.size());
	for (int j = 0; j <= mNRadius; ++j) {
		for (int i = 0; i < mNPhi; ++i) {
			int ind = j * mNPhi + i;

			Vector3f v = Vector3f(vertices[ind * 3], vertices[ind * 3 + 1], vertices[ind * 3 + 2]);
			float val  = v.norm();
			if (maxValue < 1)
				val /= maxValue;

			if (val <= 1)
				val *= 0.5f;
			else
				val = (val / maxValue) * 0.5f + 0.5f;

			appendC(colormap(val));
		}
	}

	///////////////////// Renormalize z
	if (mNormalize)
		for (auto& v : vertices)
			v /= maxValue;

	///////////////////// Indices
	std::vector<uint32> indices;

	for (int j = 0; j < mNRadius; ++j) {
		for (int i = 0; i < mNPhi; ++i) {
			uint32 row1 = j * mNPhi;
			uint32 row2 = (j + 1) * mNPhi;
			uint32 ni	= (i + 1) % mNPhi;

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

	//////////////////// Commit
	setVertices(vertices);
	setColors(colors);
	setIndices(indices);

	requestRebuild();
}
} // namespace UI
} // namespace PR