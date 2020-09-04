#include "Triangulation.h"
namespace PR {
namespace Triangulation {
// A tetrahedron consists of four vertices but we generate a group of them
constexpr uint32 STP0_INDEX = std::numeric_limits<uint32>::max();
constexpr uint32 STP1_INDEX = std::numeric_limits<uint32>::max() - 1;
constexpr uint32 STP2_INDEX = std::numeric_limits<uint32>::max() - 2;
constexpr uint32 STP3_INDEX = std::numeric_limits<uint32>::max() - 3;
constexpr uint32 STP4_INDEX = std::numeric_limits<uint32>::max() - 4;
constexpr uint32 STP5_INDEX = std::numeric_limits<uint32>::max() - 5;
constexpr uint32 STP6_INDEX = std::numeric_limits<uint32>::max() - 6;
constexpr uint32 STP7_INDEX = std::numeric_limits<uint32>::max() - 7;
constexpr uint32 STP_START	= STP7_INDEX;

struct Tetrahedron;
struct Face {
	uint32 IV0;
	uint32 IV1;
	uint32 IV2;
	Tetrahedron* Neighbor;

	inline bool hasVertex(uint32 i) const { return IV0 == i || IV1 == i || IV2 == i; }
};

inline bool operator==(const Face& a, const Face& b)
{
	return a.hasVertex(b.IV0) && a.hasVertex(b.IV1) && a.hasVertex(b.IV2);
}

struct Tetrahedron {
	uint32 IV0;
	uint32 IV1;
	uint32 IV2;
	uint32 IV3;

	Vector3f Circum;
	float CircumRadius2;

	Face F0;
	Face F1;
	Face F2;
	Face F3;

	Tetrahedron(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& v3, uint32 iv0, uint32 iv1, uint32 iv2, uint32 iv3)
		: IV0(iv0)
		, IV1(iv1)
		, IV2(iv2)
		, IV3(iv3)
	{
		// Setup faces
		F0 = Face{ IV1, IV3, IV2, nullptr };
		F1 = Face{ IV0, IV2, IV3, nullptr };
		F2 = Face{ IV0, IV3, IV1, nullptr };
		F3 = Face{ IV0, IV1, IV2, nullptr };

		// Calculate circum sphere
		const float nv0 = v0.squaredNorm();
		const float nv1 = v1.squaredNorm();
		const float nv2 = v2.squaredNorm();
		const float nv3 = v3.squaredNorm();

		const Vector3f B = 0.5f * Vector3f(nv1 - nv0, nv2 - nv0, nv3 - nv0);
		Eigen::Matrix3f A;
		A.row(0) = v1 - v0;
		A.row(1) = v2 - v0;
		A.row(2) = v3 - v0;

		Circum		  = A.inverse() * B; // I doubt that the inverse function could be further optimized (without huge time investment)
		CircumRadius2 = (v0 - Circum).squaredNorm();
	}

	inline bool contains(const Vector3f& p) const
	{
		return (p - Circum).squaredNorm() <= CircumRadius2;
	}

	inline bool isSuper() const
	{
		return IV0 >= STP_START || IV1 >= STP_START || IV2 >= STP_START || IV3 >= STP_START;
	}

	inline void makeBad()
	{
		IV1 = IV0;
		IV2 = IV0;
		IV3 = IV0;
	}

	inline bool isBad() const
	{
		return IV1 == IV0 && IV2 == IV0 && IV3 == IV0;
	}

	inline void setupNeighbors(std::vector<Tetrahedron>& others)
	{
		for (auto& t : others) {
			if (this == &t)
				continue;
			setupNeighbors(t, t.F0);
			setupNeighbors(t, t.F1);
			setupNeighbors(t, t.F2);
			setupNeighbors(t, t.F3);
		}
	}

	inline void setupNeighbors(Tetrahedron& t, Face& f)
	{
		if (!f.Neighbor) {
			if (f == F0) {
				F0.Neighbor = &t;
				f.Neighbor	= this;
			} else if (f == F1) {
				F1.Neighbor = &t;
				f.Neighbor	= this;
			} else if (f == F2) {
				F2.Neighbor = &t;
				f.Neighbor	= this;
			} else if (f == F3) {
				F3.Neighbor = &t;
				f.Neighbor	= this;
			}
		}
	}
};

std::vector<Triangle> triangulate3D(const std::vector<Vector3f>& vertices, bool surfaceOnly)
{
	if (vertices.size() < 3)
		return std::vector<Triangle>();

	// Calculate super tetrahedron for a cube given by the AABB
	Vector3f min = vertices[0];
	Vector3f max = min;
	for (const Vector3f& p : vertices) {
		min = min.cwiseMin(p);
		max = max.cwiseMax(p);
	}

	// Expand hull to make sure no intersection with the actual geometry is given
	max += Vector3f::Ones();
	min -= Vector3f::Ones();

	// Define corner vertices
	const Vector3f stp0 = min;								// 000
	const Vector3f stp1 = Vector3f(max(0), min(1), min(2)); // 100
	const Vector3f stp2 = Vector3f(min(0), max(1), min(2)); // 010
	const Vector3f stp3 = Vector3f(max(0), max(1), min(2)); // 110
	const Vector3f stp4 = Vector3f(min(0), min(1), max(2)); // 001
	const Vector3f stp5 = Vector3f(max(0), min(1), max(2)); // 101
	const Vector3f stp6 = Vector3f(min(0), max(1), max(2)); // 011
	const Vector3f stp7 = max;								// 111

	// Define actual tetrahedrons
	std::vector<Tetrahedron> tetrahedrons;
	tetrahedrons.emplace_back(stp1, stp2, stp0, stp6, STP1_INDEX, STP2_INDEX, STP0_INDEX, STP6_INDEX).setupNeighbors(tetrahedrons);
	tetrahedrons.emplace_back(stp3, stp1, stp7, stp6, STP3_INDEX, STP1_INDEX, STP7_INDEX, STP6_INDEX).setupNeighbors(tetrahedrons);
	tetrahedrons.emplace_back(stp3, stp2, stp1, stp6, STP3_INDEX, STP2_INDEX, STP1_INDEX, STP6_INDEX).setupNeighbors(tetrahedrons);
	tetrahedrons.emplace_back(stp1, stp5, stp7, stp6, STP1_INDEX, STP5_INDEX, STP7_INDEX, STP6_INDEX).setupNeighbors(tetrahedrons);
	tetrahedrons.emplace_back(stp1, stp0, stp5, stp6, STP1_INDEX, STP0_INDEX, STP5_INDEX, STP6_INDEX).setupNeighbors(tetrahedrons);
	tetrahedrons.emplace_back(stp4, stp5, stp0, stp6, STP4_INDEX, STP5_INDEX, STP0_INDEX, STP6_INDEX).setupNeighbors(tetrahedrons);

	// Query to get points. Could be made faster by modifying vertices but this would require more bandwidth
	const auto point = [&](size_t k) {
		// This is unlikely the longer the process continues
		if (PR_UNLIKELY(k >= STP_START)) {
			switch (k) {
			case STP0_INDEX:
				return stp0;
			case STP1_INDEX:
				return stp1;
			case STP2_INDEX:
				return stp2;
			case STP3_INDEX:
				return stp3;
			case STP4_INDEX:
				return stp4;
			case STP5_INDEX:
				return stp5;
			case STP6_INDEX:
				return stp6;
			case STP7_INDEX:
				return stp7;
			default:
				return vertices[k];
			}
		} else {
			return vertices[k];
		}
	};

	PR_UNUSED(point);
	PR_UNUSED(surfaceOnly);
	// TODO Add list of faces/triangles with neighbor information regarding tetrahedrons
	// - Apply same approach as delaunay
	for (size_t i = 0; i < vertices.size(); ++i) {
		const Vector3f p = vertices[i];
		PR_UNUSED(p);

		// Construct polygon

		// Erase bad tetrahedrons
		tetrahedrons.erase(std::remove_if(tetrahedrons.begin(), tetrahedrons.end(),
										  [](const Tetrahedron& t) {
											  return t.isBad();
										  }),
						   tetrahedrons.end());

		// Iterate over faces and check

		// Construct new tetrahedrons from good faces
	}

	// Remove bad and super tetrahedrons
	tetrahedrons.erase(std::remove_if(tetrahedrons.begin(), tetrahedrons.end(),
									  [](const Tetrahedron& t) {
										  return t.isBad() || t.isSuper();
									  }),
					   tetrahedrons.end());

	if (surfaceOnly) {
		std::vector<Triangle> triangles;
		const auto addFace = [&](const Face& f) {
			triangles.push_back(Triangle{ f.IV0, f.IV1, f.IV2 });
		};

		for (const auto& t : tetrahedrons) {
			if (!t.F0.Neighbor)
				addFace(t.F0);
			if (!t.F1.Neighbor)
				addFace(t.F1);
			if (!t.F2.Neighbor)
				addFace(t.F2);
			if (!t.F3.Neighbor)
				addFace(t.F3);
		}
		return triangles;
	} else {
		// TODO
		return std::vector<Triangle>();
	}
}
} // namespace Triangulation
} // namespace PR