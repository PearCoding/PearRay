#include "Hash.h"
#include "Triangulation.h"

#include <array>
#include <unordered_set>

namespace PR {
namespace Triangulation {
// A tetrahedron consists of four vertices but we generate a group of them, such that a AABB cube is filled
constexpr uint32 STP0_INDEX = std::numeric_limits<uint32>::max();
constexpr uint32 STP1_INDEX = std::numeric_limits<uint32>::max() - 1;
constexpr uint32 STP2_INDEX = std::numeric_limits<uint32>::max() - 2;
constexpr uint32 STP3_INDEX = std::numeric_limits<uint32>::max() - 3;
constexpr uint32 STP4_INDEX = std::numeric_limits<uint32>::max() - 4;
constexpr uint32 STP5_INDEX = std::numeric_limits<uint32>::max() - 5;
constexpr uint32 STP6_INDEX = std::numeric_limits<uint32>::max() - 6;
constexpr uint32 STP7_INDEX = std::numeric_limits<uint32>::max() - 7;
constexpr uint32 STP_START	= STP7_INDEX;

// Care about the orientation of a tetrahedron
//#define TETRAHEDRON_ORIENTATION /* TODO */

struct Tetrahedron;
using TetrahedronPtr = std::unique_ptr<Tetrahedron>;
struct Face {
	uint32 IV0;
	uint32 IV1;
	uint32 IV2;
	Tetrahedron* Neighbor = nullptr;
	bool Bad;

	inline bool hasVertex(uint32 k) const { return IV0 == k || IV1 == k || IV2 == k; }
	inline bool isValid() const { return IV0 != IV1 && IV2 != IV1 && IV0 != IV2; }

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
	PR_ASSERT(a.isValid() && b.isValid(), "Expected valid faces");
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

inline float volumeTetrahedron(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& v3)
{
	return std::abs((v3 - v0).dot((v2 - v0).cross(v1 - v0))) / 6;
}

inline Vector3f circumCenterTetrahedron(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& v3)
{
	const float nv0 = v0.squaredNorm();
	const float nv1 = v1.squaredNorm();
	const float nv2 = v2.squaredNorm();
	const float nv3 = v3.squaredNorm();

	const Vector3f B = 0.5f * Vector3f(nv1 - nv0, nv2 - nv0, nv3 - nv0);
	/*const float det	 = (v1 - v0).dot((v2 - v0).cross(v3 - v0));
	Eigen::Matrix3f m;
	m.col(0) = (v2 - v0).cross(v3 - v0);
	m.col(1) = (v3 - v0).cross(v1 - v0);
	m.col(2) = (v1 - v0).cross(v2 - v0);

	return m * B / det;*/
	Eigen::Matrix3f A;
	A.row(0) = v1 - v0;
	A.row(1) = v2 - v0;
	A.row(2) = v3 - v0;
	return A.inverse() * B;
}

inline float determinantTetrahedron(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& v3)
{
	return (v1 - v0).cross(v2 - v0).dot(v3 - v0);
}

inline float determinantTriangle(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2)
{
	return (v1 - v0).cross(v2 - v1).dot(v0);
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
		, Circum(circumCenterTetrahedron(v0, v1, v2, v3))
		, CircumRadius2((v0 - Circum).squaredNorm())
	{
#ifdef TETRAHEDRON_ORIENTATION
		PR_ASSERT(determinantTetrahedron(v0, v1, v2, v3) >= 0, "Expected positive oriented tetrahedron");
#endif

		// Setup faces
		Faces[0] = Face{ IV1, IV2, IV3, nullptr, false };
		Faces[1] = Face{ IV0, IV3, IV2, nullptr, false };
		Faces[2] = Face{ IV0, IV1, IV3, nullptr, false };
		Faces[3] = Face{ IV0, IV1, IV2, nullptr, false };

#ifdef FACE_ORIENTATION
		if (determinantTriangle(v1, v2, v3) < 0)
			std::swap(Faces[0].IV1, Faces[0].IV2);

		if (determinantTriangle(v0, v3, v2) < 0)
			std::swap(Faces[1].IV1, Faces[1].IV2);

		if (determinantTriangle(v0, v1, v3) < 0)
			std::swap(Faces[2].IV1, Faces[2].IV2);

		if (determinantTriangle(v0, v1, v2) < 0)
			std::swap(Faces[3].IV1, Faces[3].IV2);
#endif
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

	inline Face* getAdjacentFace(const Face& f)
	{
		for (size_t j = 0; j < Faces.size(); ++j) {
			if (Faces[j] == f)
				return &Faces[j];
		}
		return nullptr;
	}

	inline void releaseNeighbor(size_t f)
	{
		releaseNeighbor(Faces[f]);
	}

	inline static void releaseNeighbor(Face& f)
	{
		if (f.Neighbor) {
			auto ptr = f.Neighbor->getAdjacentFace(f);
			PR_ASSERT(ptr, "Adjacent relationship is broken");
			ptr->Neighbor = nullptr;
			f.Neighbor	  = nullptr;
		}
	}

	inline void setNeighbor(size_t f, Tetrahedron* t)
	{
		auto ptr = t->getAdjacentFace(Faces[f]);
		if (ptr) {
			releaseNeighbor(f);
			releaseNeighbor(*ptr);

			Faces[f].Neighbor = t;
			ptr->Neighbor	  = this;
		}
	}

	inline void setupNeighbors(std::vector<TetrahedronPtr>& others)
	{
		for (size_t i = 0; i < Faces.size(); ++i) {
			if (!Faces[i].Neighbor) { // Only setup still-empty neighbors
				for (auto it = others.begin(); it != others.end(); ++it) {
					Tetrahedron* t = it->get();
					Face* f		   = t->getAdjacentFace(Faces[i]);
					if (f) {
						t->releaseNeighbor(*f);
						Faces[i].Neighbor = t;
						f->Neighbor		  = this;
						break;
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

// Tiny optimization -> As we now that the fourth face is defined without p we can reuse the existing neighbor relationship
inline static void addTetrahedron(std::vector<TetrahedronPtr>& list, const Face& f,
								  const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& p,
								  /*iv0,iv1 and iv2 is given by f*/ uint32 ip)
{
#ifdef TETRAHEDRON_ORIENTATION
	// TODO: Probably using a better face orientation, this step could be skipped
	std::unique_ptr<Tetrahedron> ptr = determinantTetrahedron(v0, v1, v2, p) >= 0
										   ? std::make_unique<Tetrahedron>(v0, v1, v2, p, f.IV0, f.IV1, f.IV2, ip)
										   : std::make_unique<Tetrahedron>(v0, v2, v1, p, f.IV0, f.IV2, f.IV1, ip);
#else
	auto ptr = std::make_unique<Tetrahedron>(v0, v1, v2, p, f.IV0, f.IV1, f.IV2, ip);
#endif

	if (f.Neighbor)
		ptr->setNeighbor(3, f.Neighbor);
	ptr->setupNeighbors(list);
	list.emplace_back(std::move(ptr));
}

// TODO Handle degeneracies and optimize data structure use
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
	const Vector3f stp0 = min;								// 000 (A)
	const Vector3f stp1 = Vector3f(max(0), min(1), min(2)); // 100 (B)
	const Vector3f stp2 = Vector3f(min(0), max(1), min(2)); // 010 (D)
	const Vector3f stp3 = Vector3f(max(0), max(1), min(2)); // 110 (C)
	const Vector3f stp4 = Vector3f(min(0), min(1), max(2)); // 001 (E)
	const Vector3f stp5 = Vector3f(max(0), min(1), max(2)); // 101 (F)
	const Vector3f stp6 = Vector3f(min(0), max(1), max(2)); // 011 (H)
	const Vector3f stp7 = max;								// 111 (G)

#define _INIT_TETR(a, b, c, d) \
	addTetrahedron(tetrahedrons, stp##a, stp##b, stp##c, stp##d, STP##a##_INDEX, STP##b##_INDEX, STP##c##_INDEX, STP##d##_INDEX)

	// Define five tetrahedrons to fill the bounding box
	// See https://www.geogebra.org/m/CcXzmKtR for an example
	std::vector<TetrahedronPtr> tetrahedrons;
	_INIT_TETR(0, 1, 3, 5); // ABCF
	_INIT_TETR(3, 6, 5, 7); // CHFG
	_INIT_TETR(0, 6, 3, 2); // AHCD
	_INIT_TETR(5, 0, 4, 6); // FAEH
	_INIT_TETR(0, 3, 6, 5); // ACHF

#undef _INIT_TETR

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

	std::vector<Face> faces; // Temporary face stack
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
		for (auto it1 = faces.begin(); it1 != faces.end(); ++it1) {
			if (it1->isBad())
				continue;
			for (auto it2 = it1 + 1; it2 != faces.end(); ++it2) {
				if (*it1 == *it2) {
					it1->makeBad();
					it2->makeBad();
					//break; // Only one pair available, so stop if found
				}
			}
		}

		// Construct new tetrahedrons from good faces only
		for (const auto& f : faces) {
			if (!f.isBad())
				addTetrahedron(tetrahedrons, f, point(f.IV0), point(f.IV1), point(f.IV2), p, i);
		}
	}
	std::vector<Face>().swap(faces); // Clean up

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
			for (size_t j = 0; j < t->Faces.size(); ++j) {
				if (!t->Faces[j].Neighbor)
					addFace(t->Faces[j]);
			}
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