#include "Delaunay.h"
namespace PR {
namespace Delaunay {
// This can be further optimized!
// Based on https://en.wikipedia.org/wiki/Bowyer%E2%80%93Watson_algorithm

struct PR_LIB_BASE InternalTriangle2D {
	Vector2f V0;
	Vector2f V1;
	Vector2f V2;
	uint32 IV0;
	uint32 IV1;
	uint32 IV2;

	Vector2f Circum;
	bool Bad;

	InternalTriangle2D(const Vector2f& v0, const Vector2f& v1, const Vector2f& v2,
					   uint32 iv0, uint32 iv1, uint32 iv2)
		: V0(v0)
		, V1(v1)
		, V2(v2)
		, IV0(iv0)
		, IV1(iv1)
		, IV2(iv2)
		, Circum()
		, Bad(false)
	{
		const float nv0 = V0.squaredNorm();
		const float nv1 = V1.squaredNorm();
		const float nv2 = V2.squaredNorm();

		Circum(0) = (nv0 * (V2(1) - V1(1))
					 + nv1 * (V0(1) - V2(1))
					 + nv2 * (V1(1) - V0(1)))
					/ (V0(0) * (V2(1) - V1(1)) + V1(0) * (V0(1) - V2(1)) + V2(0) * (V1(1) - V0(1)));
		Circum(1) = (nv0 * (V2(0) - V1(0))
					 + nv1 * (V0(0) - V2(0))
					 + nv2 * (V1(0) - V0(0)))
					/ (V0(1) * (V2(0) - V1(0)) + V1(1) * (V0(0) - V2(0)) + V2(1) * (V1(0) - V0(0)));

		Circum /= 2;
	}

	inline bool contains(const Vector2f& p) const
	{
		return (p - Circum).squaredNorm() <= (V0 - Circum).squaredNorm();
	}
};

struct Edge2D {
	Vector2f V0;
	Vector2f V1;
	uint32 IV0;
	uint32 IV1;
	bool Bad;
};

inline bool edgeEqual(const Edge2D& a, const Edge2D& b)
{
	return (a.V0 == b.V0 && a.V1 == b.V1) || (a.V0 == b.V1 && a.V1 == b.V0);
}

constexpr uint32 STP0_INDEX = std::numeric_limits<uint32>::max();
constexpr uint32 STP1_INDEX = std::numeric_limits<uint32>::max() - 1;
constexpr uint32 STP2_INDEX = std::numeric_limits<uint32>::max() - 2;
std::vector<Triangle> triangulate2D(const std::vector<Vector2f>& vertices)
{
	// Calculate super triangle
	Vector2f min = vertices[0];
	Vector2f max = min;
	for (const Vector2f& p : vertices) {
		min = min.cwiseMin(p);
		max = max.cwiseMax(p);
	}

	const Vector2f d	 = max - min;
	const float deltaMax = d.maxCoeff();
	const Vector2f mid	 = (max + min) / 2;

	const Vector2f stp0 = Vector2f(mid(0) - 25 * deltaMax, mid(1) - deltaMax);
	const Vector2f stp1 = Vector2f(mid(0), mid(1) + 25 * deltaMax);
	const Vector2f stp2 = Vector2f(mid(0) + 25 * deltaMax, mid(1) - deltaMax);

	// Start with the super triangle
	std::vector<InternalTriangle2D> triangles;
	triangles.push_back(InternalTriangle2D(stp0, stp1, stp2, STP0_INDEX, STP1_INDEX, STP2_INDEX));

	std::vector<Edge2D> edges; // temporary
	for (size_t i = 0; i < vertices.size(); ++i) {
		const Vector2f p = vertices[i];

		// Construct polygon
		edges.clear();
		for (auto& t : triangles) {
			if (t.contains(p)) {
				t.Bad = true;
				edges.push_back(Edge2D{ t.V0, t.V1, t.IV0, t.IV1, false });
				edges.push_back(Edge2D{ t.V1, t.V2, t.IV1, t.IV2, false });
				edges.push_back(Edge2D{ t.V2, t.V0, t.IV2, t.IV0, false });
			}
		}

		// Erase bad triangles
		triangles.erase(std::remove_if(triangles.begin(), triangles.end(),
									   [](const InternalTriangle2D& t) { return t.Bad; }),
						triangles.end());

		// Iterate over edges
		for (auto it1 = edges.begin(); it1 != edges.end(); ++it1) {
			for (auto it2 = it1 + 1; it2 != edges.end(); ++it2) {
				if (edgeEqual(*it1, *it2)) {
					it1->Bad = true;
					it2->Bad = true;
				}
			}
		}

		// Construct triangles from good edges
		for (const auto& e : edges) {
			if (!e.Bad)
				triangles.push_back(InternalTriangle2D(e.V0, e.V1, p, e.IV0, e.IV1, i));
		}
	}

	// Remove super triangle parts from list
	triangles.erase(std::remove_if(triangles.begin(), triangles.end(),
								   [](const InternalTriangle2D& t) {
									   return t.IV0 == STP0_INDEX || t.IV0 == STP1_INDEX || t.IV0 == STP2_INDEX
											  || t.IV1 == STP0_INDEX || t.IV1 == STP1_INDEX || t.IV1 == STP2_INDEX
											  || t.IV2 == STP0_INDEX || t.IV2 == STP1_INDEX || t.IV2 == STP2_INDEX;
								   }),
					triangles.end());

	// Map to optimized triangle format and return
	std::vector<Triangle> opt_triangles;
	opt_triangles.reserve(triangles.size());
	for (const auto& t : triangles)
		opt_triangles.push_back(Triangle{ t.IV0, t.IV1, t.IV2 });
	return opt_triangles;
}
} // namespace Delaunay
} // namespace PR