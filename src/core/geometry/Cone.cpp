#include "Cone.h"
#include "Disk.h"
#include "Triangle.h"
#include "math/Tangent.h"

namespace PR {
// First two entries are base and tip
void Cone::triangulate(const Vector3f& basePos, const Vector3f& tipPos, float baseRadius, uint32 sectionCount, std::vector<float>& vertices)
{
	sectionCount = std::max<uint32>(3, sectionCount);

	Vector3f D = tipPos - basePos;
	Vector3f N = D.normalized();
	Vector3f Nx, Ny;
	Tangent::frame(N, Nx, Ny);

	auto add = [&](const Vector3f& v) {vertices.push_back(v[0]); vertices.push_back(v[1]); vertices.push_back(v[2]); };

	add(basePos);
	add(tipPos);

	float step = 1.0f / sectionCount;
	for (uint32 i = 0; i < sectionCount; ++i) {
		float x = baseRadius * std::cos(2 * 3.141592f * step * i);
		float y = baseRadius * std::sin(2 * 3.141592f * step * i);

		add(Nx * x + Ny * y + basePos);
	}
}

void Cone::triangulateIndices(uint32 baseID, uint32 tipID, uint32 sectionCount, std::vector<uint32>& indices, uint32 off)
{
	sectionCount = std::max<uint32>(3, sectionCount);

	for (uint32 i = 0; i < sectionCount; ++i) {
		uint32 C  = i + off;
		uint32 NC = (i + 1 < sectionCount ? i + 1 : 0) + off;

		Triangle::triangulateIndices({ baseID, NC, C }, indices);
		Triangle::triangulateIndices({ tipID, C, NC }, indices);
	}
}
} // namespace PR
