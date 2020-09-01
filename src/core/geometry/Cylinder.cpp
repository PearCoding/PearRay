#include "Cylinder.h"
#include "Disk.h"
#include "Plane.h"
#include "Triangle.h"
#include "math/Tangent.h"

namespace PR {
// First two entries are base1 and base2
void Cylinder::triangulate(const Vector3f& base1Pos, const Vector3f& base2Pos, float topRadius, float bottomRadius, uint32 sectionCount, std::vector<float>& vertices)
{
	sectionCount = std::max<uint32>(3, sectionCount);

	Vector3f D = base2Pos - base1Pos;
	Vector3f N = D.normalized();
	Vector3f Nx, Ny;
	Tangent::frame(N, Nx, Ny);

	auto add = [&](const Vector3f& v) {vertices.push_back(v[0]); vertices.push_back(v[1]); vertices.push_back(v[2]); };

	add(base1Pos);
	add(base2Pos);

	float step = 1.0f / sectionCount;
	for (uint32 i = 0; i < sectionCount; ++i) {
		float x = std::cos(2 * 3.141592f * step * i);
		float y = std::sin(2 * 3.141592f * step * i);

		add(bottomRadius * Nx * x + bottomRadius * Ny * y + base1Pos);
		add(topRadius * Nx * x + topRadius * Ny * y + base2Pos);
	}
}

void Cylinder::triangulateIndices(uint32 base1ID, uint32 base2ID, uint32 sectionCount, std::vector<uint32>& indices, uint32 off)
{
	sectionCount = std::max<uint32>(3, sectionCount);

	for (uint32 i = 0; i < sectionCount; ++i) {
		uint32 C1  = 2 * i + off;
		uint32 NC1 = 2 * (i + 1 < sectionCount ? i + 1 : 0) + off;
		uint32 C2  = C1 + 1;
		uint32 NC2 = NC1 + 1;

		Triangle::triangulateIndices({ base1ID, NC1, C1 }, indices);
		Triangle::triangulateIndices({ base2ID, C2, NC2 }, indices);
		Plane::triangulateIndices({ C1, NC1, NC2, C2 }, indices);
	}
}
} // namespace PR
