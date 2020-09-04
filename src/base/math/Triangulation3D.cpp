#include "Hash.h"
#include "Triangulation.h"

#include <unordered_set>

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
using TetrahedronPtr = std::unique_ptr<Tetrahedron>;
struct Face {
	uint32 IV0;
	uint32 IV1;
	uint32 IV2;
	Tetrahedron* Neighbor = nullptr;
	bool Bad;

	inline bool hasVertex(uint32 i) const { return IV0 == i || IV1 == i || IV2 == i; }

	inline void makeBad()
	{
		Bad = true;
	}

	inline bool isBad() const
	{
		return Bad;
	}
};

inline bool operator==(const Face& a, const Face& b)
{
	return a.hasVertex(b.IV0) && a.hasVertex(b.IV1) && a.hasVertex(b.IV2);
}

struct FaceHash {
	std::size_t operator()(Face const& s) const noexcept
	{
		std::size_t h1 = hash_union(std::hash<uint32>{}(s.IV0), std::hash<uint32>{}(s.IV1));
		hash_combine(h1, std::hash<uint32>{}(s.IV2));
		return h1;
	}
};

inline float tetrahedronVolume(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& v3)
{
	return std::abs((v3 - v0).dot((v2 - v0).cross(v1 - v0))) / 6;
}

struct Tetrahedron {
	uint32 IV0;
	uint32 IV1;
	uint32 IV2;
	uint32 IV3;

	Vector3f Circum;
	float CircumRadius2;

	std::array<Face, 4> Faces;

	inline Tetrahedron(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& v3, uint32 iv0, uint32 iv1, uint32 iv2, uint32 iv3)
		: IV0(iv0)
		, IV1(iv1)
		, IV2(iv2)
		, IV3(iv3)
	{
		// Setup faces
		Faces[0] = Face{ IV1, IV3, IV2, nullptr, false };
		Faces[1] = Face{ IV0, IV2, IV3, nullptr, false };
		Faces[2] = Face{ IV0, IV3, IV1, nullptr, false };
		Faces[3] = Face{ IV0, IV1, IV2, nullptr, false };

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

	inline bool isSuper() const
	{
		return IV0 >= STP_START || IV1 >= STP_START || IV2 >= STP_START || IV3 >= STP_START;
	}

	inline Face* getAdjacentFace(Tetrahedron* t)
	{
		for (size_t j = 0; j < Faces.size(); ++j) {
			if (Faces[j].Neighbor == t)
				return &Faces[j];
		}
		return nullptr;
	}

	inline void releaseNeighbor(size_t f)
	{
		if (Faces[f].Neighbor) {
			auto ptr = Faces[f].Neighbor->getAdjacentFace(this);
			//PR_ASSERT(ptr, "Adjacent relationship is broken");
			if (ptr)
				ptr->Neighbor = nullptr;
			Faces[f].Neighbor = nullptr;
		}
	}

	inline void setNeighbor(size_t f, Tetrahedron* t)
	{
		for (size_t j = 0; j < Faces.size(); ++j) {
			if (t->Faces[j] == Faces[f]) {
				releaseNeighbor(f);
				t->releaseNeighbor(j);

				Faces[f].Neighbor	 = t;
				t->Faces[j].Neighbor = this;
				break;
			}
		}
	}

	inline void setupNeighbors(std::vector<TetrahedronPtr>& others)
	{
		for (size_t i = 0; i < Faces.size(); ++i) {
		NeighborEndLoop:
			if (!Faces[i].Neighbor) { // Only setup still-empty neighbors
				for (auto it = others.begin(); it != others.end(); ++it) {
					Tetrahedron* t = it->get();

					for (size_t j = 0; j < Faces.size(); ++j) {
						if (t->Faces[j] == Faces[i]) {
							t->releaseNeighbor(j);
							Faces[i].Neighbor	 = t;
							t->Faces[j].Neighbor = this;
							goto NeighborEndLoop;
						}
					}
				}
			}
		}
	}

	inline void disconnectFromNeighbors()
	{
		for (size_t i = 0; i < Faces.size(); ++i)
			releaseNeighbor(i);
	}
};

static void checkAndAdd(Tetrahedron* t, const Vector3f& p, std::vector<Face>& faces)
{
	if (!t->isBad() && t->contains(p)) {
		for (size_t j = 0; j < t->Faces.size(); ++j)
			faces.push_back(t->Faces[j]);
		t->makeBad();

		for (size_t j = 0; j < t->Faces.size(); ++j) {
			if (t->Faces[j].Neighbor)
				checkAndAdd(t->Faces[j].Neighbor, p, faces);
		}
	}
}

inline static void addTetrahedron(std::vector<TetrahedronPtr>& list,
								  const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& v3,
								  uint32 iv0, uint32 iv1, uint32 iv2, uint32 iv3)
{
	auto ptr = std::make_unique<Tetrahedron>(v0, v1, v2, v3, iv0, iv1, iv2, iv3);
	ptr->setupNeighbors(list);
	list.emplace_back(std::move(ptr));
}

// Tiny optimization -> As we now that the fourth face is defined without p we can reuse the existent neighbor relationship
inline static void addTetrahedron(std::vector<TetrahedronPtr>& list, const Face& f,
								  const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& p,
								  /*iv0,iv1 and iv2 is given by f*/ uint32 ip)
{
	auto ptr = std::make_unique<Tetrahedron>(v0, v1, v2, p, f.IV0, f.IV1, f.IV2, ip);
	if (f.Neighbor)
		ptr->setNeighbor(3, f.Neighbor);
	ptr->setupNeighbors(list);
	list.emplace_back(std::move(ptr));
}

// TODO Handle degeneracies
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
	std::vector<TetrahedronPtr> tetrahedrons;
	addTetrahedron(tetrahedrons, stp1, stp2, stp0, stp6, STP1_INDEX, STP2_INDEX, STP0_INDEX, STP6_INDEX);
	addTetrahedron(tetrahedrons, stp3, stp1, stp7, stp6, STP3_INDEX, STP1_INDEX, STP7_INDEX, STP6_INDEX);
	addTetrahedron(tetrahedrons, stp3, stp2, stp1, stp6, STP3_INDEX, STP2_INDEX, STP1_INDEX, STP6_INDEX);
	addTetrahedron(tetrahedrons, stp1, stp5, stp7, stp6, STP1_INDEX, STP5_INDEX, STP7_INDEX, STP6_INDEX);
	addTetrahedron(tetrahedrons, stp1, stp0, stp5, stp6, STP1_INDEX, STP0_INDEX, STP5_INDEX, STP6_INDEX);
	addTetrahedron(tetrahedrons, stp4, stp5, stp0, stp6, STP4_INDEX, STP5_INDEX, STP0_INDEX, STP6_INDEX);

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

	std::vector<Face> faces;
	for (size_t i = 0; i < vertices.size(); ++i) {
		const Vector3f p = vertices[i];

		// Construct polygon
		faces.clear();
		for (auto it = tetrahedrons.begin(); it != tetrahedrons.end(); ++it) {
			Tetrahedron* t = it->get();
			if (t->contains(p)) { // Found the starting point!
				for (size_t j = 0; j < t->Faces.size(); ++j)
					faces.push_back(t->Faces[j]);
				t->makeBad();

				for (size_t j = 0; j < t->Faces.size(); ++j) {
					if (t->Faces[j].Neighbor)
						checkAndAdd(t->Faces[j].Neighbor, p, faces);
				}
				break;
			}
		}

		// Erase bad tetrahedrons
		for (auto it = tetrahedrons.begin(); it != tetrahedrons.end();) {
			if (it->get()->isBad()) {
				it->get()->disconnectFromNeighbors();
				it = tetrahedrons.erase(it);
			} else {
				++it;
			}
		}

		// Iterate over faces and check
		// FIXME: Probably this is the place where everything goes wrong
		for (auto it1 = faces.begin(); it1 != faces.end(); ++it1) {
			if (it1->isBad())
				continue;
			for (auto it2 = it1 + 1; it2 != faces.end(); ++it2) {
				if (*it1 == *it2) {
					it1->makeBad();
					it2->makeBad();
				}
			}
		}

		// Construct new tetrahedrons from good faces only
		for (const auto& f : faces) {
			if (!f.isBad())
				addTetrahedron(tetrahedrons, f, point(f.IV0), point(f.IV1), point(f.IV2), p, i);
		}
	}

	// Remove bad and super tetrahedrons
	for (auto it = tetrahedrons.begin(); it != tetrahedrons.end();) {
		if (it->get()->isBad() || it->get()->isSuper()) {
			it->get()->disconnectFromNeighbors();
			it = tetrahedrons.erase(it);
		} else {
			++it;
		}
	}

	// Construct triangle list based on given operation mode
	std::vector<Triangle> triangles;
	const auto addFace = [&](const Face& f) {
		triangles.push_back(Triangle{ f.IV0, f.IV1, f.IV2 });
	};

	if (surfaceOnly) {
		for (auto it = tetrahedrons.begin(); it != tetrahedrons.end(); ++it) {
			Tetrahedron* t = it->get();
			for (size_t j = 0; j < t->Faces.size(); ++j)
				if (!t->Faces[j].Neighbor)
					addFace(t->Faces[j]);
		}
	} else {
		std::unordered_set<Face, FaceHash> unique_faces;
		for (auto it = tetrahedrons.begin(); it != tetrahedrons.end(); ++it) {
			Tetrahedron* t = it->get();
			for (size_t j = 0; j < t->Faces.size(); ++j)
				unique_faces.insert(t->Faces[j]);
		}

		for (const auto& f : unique_faces)
			addFace(f);
	}
	return triangles;
}
} // namespace Triangulation
} // namespace PR