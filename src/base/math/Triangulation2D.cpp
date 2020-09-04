#include "Triangulation.h"
namespace PR {
namespace Triangulation {
// This can be further optimized!
// Based on https://en.wikipedia.org/wiki/Bowyer%E2%80%93Watson_algorithm

struct Circum2D {
	Vector2f Circum;
	float CircumRadius2;

	Circum2D(const Vector2f& v0, const Vector2f& v1, const Vector2f& v2)
		: Circum()
	{
		const float nv0 = v0.squaredNorm();
		const float nv1 = v1.squaredNorm();
		const float nv2 = v2.squaredNorm();

		Circum(0) = (nv0 * (v2(1) - v1(1))
					 + nv1 * (v0(1) - v2(1))
					 + nv2 * (v1(1) - v0(1)))
					/ (v0(0) * (v2(1) - v1(1)) + v1(0) * (v0(1) - v2(1)) + v2(0) * (v1(1) - v0(1)));
		Circum(1) = (nv0 * (v2(0) - v1(0))
					 + nv1 * (v0(0) - v2(0))
					 + nv2 * (v1(0) - v0(0)))
					/ (v0(1) * (v2(0) - v1(0)) + v1(1) * (v0(0) - v2(0)) + v2(1) * (v1(0) - v0(0)));

		Circum /= 2;

		CircumRadius2 = (v0 - Circum).squaredNorm();
	}

	inline bool contains(const Vector2f& p) const
	{
		return (p - Circum).squaredNorm() <= CircumRadius2;
	}
};

struct Edge2D {
	uint32 IV0;
	uint32 IV1;
	bool Bad;
};

inline bool edgeEqual(const Edge2D& a, const Edge2D& b)
{
	return (a.IV0 == b.IV0 && a.IV1 == b.IV1) || (a.IV0 == b.IV1 && a.IV1 == b.IV0);
}

inline void makeBad(Triangle& t)
{
	t.V1 = t.V0;
	t.V2 = t.V0;
}

inline bool isBad(const Triangle& t)
{
	return t.V1 == t.V0 && t.V2 == t.V0;
}

constexpr uint32 STP0_INDEX = std::numeric_limits<uint32>::max();
constexpr uint32 STP1_INDEX = std::numeric_limits<uint32>::max() - 1;
constexpr uint32 STP2_INDEX = std::numeric_limits<uint32>::max() - 2;
constexpr uint32 STP_START	= STP2_INDEX;
std::vector<Triangle> triangulate2D(const std::vector<Vector2f>& vertices)
{
	if (vertices.size() < 3)
		return std::vector<Triangle>();

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

	// Query to get points. Could be made faster by modifying vertices but this would require more bandwidth
	const auto point = [&](size_t k) {
		switch (k) {
		case STP0_INDEX:
			return stp0;
		case STP1_INDEX:
			return stp1;
		case STP2_INDEX:
			return stp2;
		default:
			return vertices[k];
		}
	};

	// Start with the super triangle
	std::vector<Triangle> triangles;
	std::vector<Circum2D> circums; // Contains temporary circum circle buffer
	triangles.push_back(Triangle{ STP0_INDEX, STP1_INDEX, STP2_INDEX });
	circums.push_back(Circum2D(stp0, stp1, stp2));

	std::vector<Edge2D> edges; // temporary
	for (size_t i = 0; i < vertices.size(); ++i) {
		const Vector2f p = vertices[i];

		// Construct polygon
		edges.clear();
		for (size_t k = 0; k < triangles.size(); ++k) {
			Triangle& t		  = triangles[k];
			const Circum2D& c = circums[k];
			if (c.contains(p)) {
				edges.push_back(Edge2D{ t.V0, t.V1, false });
				edges.push_back(Edge2D{ t.V1, t.V2, false });
				edges.push_back(Edge2D{ t.V2, t.V0, false });
				makeBad(t);
			}
		}

		// Erase bad triangles
		auto it2 = circums.begin();
		for (auto it = triangles.begin(); it != triangles.end();) {
			if (isBad(*it)) {
				it	= triangles.erase(it);
				it2 = circums.erase(it2);
			} else {
				++it;
				++it2;
			}
		}

		// Iterate over edges
		for (auto it1 = edges.begin(); it1 != edges.end(); ++it1) {
			for (auto it2 = it1 + 1; it2 != edges.end(); ++it2) {
				if (edgeEqual(*it1, *it2)) {
					it1->Bad = true;
					it2->Bad = true;
				}
			}
		}

		// Construct new triangles from good edges
		for (const auto& e : edges) {
			if (!e.Bad) {
				triangles.push_back(Triangle{ e.IV0, e.IV1, (uint32)i });
				circums.push_back(Circum2D(point(e.IV0), point(e.IV1), p));
			}
		}
	}

	// Remove super triangle parts from list
	triangles.erase(std::remove_if(triangles.begin(), triangles.end(),
								   [](const Triangle& t) {
									   return isBad(t) || t.V0 >= STP_START || t.V1 >= STP_START || t.V2 >= STP_START;
								   }),
					triangles.end());

	return triangles;
}
} // namespace Triangulation
} // namespace PR