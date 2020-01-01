#include "PlyLoader.h"
#include "Environment.h"
#include "Logger.h"
#include "Platform.h"
#include "SceneLoadContext.h"
#include "cache/Cache.h"
#include "mesh/MeshBase.h"
#include "mesh/TriMesh.h"

#include <climits>
#include <fstream>

// https://stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c
template <typename T>
T swap_endian(T u)
{
	static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");

	union {
		T u;
		unsigned char u8[sizeof(T)];
	} source, dest;

	source.u = u;

	for (size_t k = 0; k < sizeof(T); k++)
		dest.u8[k] = source.u8[sizeof(T) - k - 1];

	return dest.u;
}

namespace PR {
PlyLoader::PlyLoader(const std::string& name)
	: mName(name)
	, mScale(1)
	, mFlipNormal(false)
	, mCacheMode(CM_Auto)
{
}

PlyLoader::~PlyLoader()
{
}

struct Header {
	int VertexCount		  = 0;
	int FaceCount		  = 0;
	int XElem			  = -1;
	int YElem			  = -1;
	int ZElem			  = -1;
	int NXElem			  = -1;
	int NYElem			  = -1;
	int NZElem			  = -1;
	int UElem			  = -1;
	int VElem			  = -1;
	int VertexPropCount   = 0;
	int IndElem			  = -1;
	bool SwitchEndianness = false;

	inline bool hasVertices() const { return XElem >= 0 && YElem >= 0 && ZElem >= 0; }
	inline bool hasNormals() const { return NXElem >= 0 && NYElem >= 0 && NZElem >= 0; }
	inline bool hasUVs() const { return UElem >= 0 && VElem >= 0; }
	inline bool hasIndices() const { return IndElem >= 0; }
};

std::unique_ptr<MeshBase> read(std::fstream& stream, const Header& header, bool ascii)
{
	const auto readFloat = [&]() {
		float val;
		stream.read(reinterpret_cast<char*>(&val), sizeof(val));
		if (header.SwitchEndianness)
			val = swap_endian<float>(val);
		return val;
	};
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> uvs;

	vertices.reserve(header.VertexCount * 3);
	if (header.hasNormals())
		normals.reserve(header.VertexCount * 3);
	if (header.hasUVs())
		uvs.reserve(header.VertexCount * 2);

	for (int i = 0; i < header.VertexCount; ++i) {

		float x = 0, y = 0, z = 0;
		float nx = 0, ny = 0, nz = 0;
		float u = 0, v = 0;

		if (ascii) {
			std::string line;
			if (!std::getline(stream, line)) {
				PR_LOG(L_ERROR) << "Not enough vertices given" << std::endl;
				return nullptr; // Failed
			}
			std::stringstream sstream(line);
			int elem = 0;
			while (sstream) {
				float val;
				sstream >> val;

				if (header.XElem == elem)
					x = val;
				else if (header.YElem == elem)
					y = val;
				else if (header.ZElem == elem)
					z = val;
				else if (header.NXElem == elem)
					nx = val;
				else if (header.NYElem == elem)
					ny = val;
				else if (header.NZElem == elem)
					nz = val;
				else if (header.UElem == elem)
					u = val;
				else if (header.VElem == elem)
					v = val;
				++elem;
			}
		} else {
			for (int elem = 0; elem < header.VertexPropCount; ++elem) {
				float val = readFloat();
				if (header.XElem == elem)
					x = val;
				else if (header.YElem == elem)
					y = val;
				else if (header.ZElem == elem)
					z = val;
				else if (header.NXElem == elem)
					nx = val;
				else if (header.NYElem == elem)
					ny = val;
				else if (header.NZElem == elem)
					nz = val;
				else if (header.UElem == elem)
					u = val;
				else if (header.VElem == elem)
					v = val;
			}
		}

		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);

		if (header.hasNormals()) {
			float norm = sqrt(nx * nx + ny * ny + nz * nz);
			if (norm <= PR_EPSILON)
				norm = 1.0f;
			normals.push_back(nx / norm);
			normals.push_back(ny / norm);
			normals.push_back(nz / norm);
		}

		if (header.hasUVs()) {
			uvs.push_back(u);
			uvs.push_back(v);
		}
	}

	std::vector<uint32> indices;
	std::vector<uint8> vertPerFace;

	indices.reserve(header.FaceCount * 4);
	vertPerFace.reserve(header.FaceCount);

	if (ascii) {
		for (int i = 0; i < header.FaceCount; ++i) {
			std::string line;
			if (!std::getline(stream, line)) {
				PR_LOG(L_ERROR) << "Not enough indices given" << std::endl;
				return nullptr; // Failed
			}

			std::stringstream sstream(line);

			uint32 elems;
			sstream >> elems;
			for (uint32 j = 0; j < elems; ++j) {
				uint32 ind;
				sstream >> ind;
				indices.push_back(ind);
			}
			vertPerFace.push_back(elems);

			if (elems != 3 && elems != 4) {
				PR_LOG(L_ERROR) << "Only triangle or quads allowed in ply files" << std::endl;
				return nullptr;
			}
		}
	} else {
		for (int i = 0; i < header.FaceCount; ++i) {
			uint8 elems;
			stream.read(reinterpret_cast<char*>(&elems), sizeof(elems));
			for (uint32 j = 0; j < elems; ++j) {
				uint32 ind;
				stream.read(reinterpret_cast<char*>(&ind), sizeof(ind));
				if (header.SwitchEndianness)
					ind = swap_endian(ind);
				indices.push_back(ind);
			}
			vertPerFace.push_back(elems);

			if (elems != 3 && elems != 4) {
				PR_LOG(L_ERROR) << "Only triangle or quads allowed in ply files" << std::endl;
				return nullptr;
			}
		}
	}

	// Build
	std::unique_ptr<MeshBase> cnt = std::make_unique<MeshBase>();
	cnt->setVertices(vertices);
	if (header.hasNormals())
		cnt->setNormals(normals);
	if (header.hasUVs())
		cnt->setUVs(uvs);

	cnt->setIndices(indices);
	cnt->setFaceVertexCount(vertPerFace);

	if (!header.hasNormals())
		cnt->buildNormals();

	return cnt;
}

void PlyLoader::load(const std::wstring& file, const SceneLoadContext& ctx)
{
	std::fstream stream(encodePath(file), std::ios::in | std::ios::binary);
	if (!stream)
		return;

	// Header
	std::string magic;
	stream >> magic;
	if (magic != "ply") {
		PR_LOG(L_WARNING) << "Given file is not a ply file." << std::endl;
		return;
	}

	std::string method;
	Header header;

	int facePropCounter = 0;
	for (std::string line; std::getline(stream, line);) {
		std::stringstream sstream(line);

		std::string action;
		sstream >> action;
		if (action == "comment")
			continue;
		else if (action == "format") {
			sstream >> method;
		} else if (action == "element") {
			std::string type;
			sstream >> type;
			if (type == "vertex")
				sstream >> header.VertexCount;
			else if (type == "face")
				sstream >> header.FaceCount;
		} else if (action == "property") {
			std::string type;
			sstream >> type;
			if (type == "float") {
				std::string name;
				sstream >> name;
				if (name == "x")
					header.XElem = header.VertexPropCount;
				else if (name == "y")
					header.YElem = header.VertexPropCount;
				else if (name == "z")
					header.ZElem = header.VertexPropCount;
				else if (name == "nx")
					header.NXElem = header.VertexPropCount;
				else if (name == "ny")
					header.NYElem = header.VertexPropCount;
				else if (name == "nz")
					header.NZElem = header.VertexPropCount;
				else if (name == "u")
					header.UElem = header.VertexPropCount;
				else if (name == "v")
					header.VElem = header.VertexPropCount;
				++header.VertexPropCount;
			} else if (type == "list") {
				++facePropCounter;

				std::string countType;
				sstream >> countType;

				std::string indType;
				sstream >> indType;

				std::string name;
				sstream >> name;
				if (countType != "uchar" && indType != "int" && indType != "uint8") {
					PR_LOG(L_WARNING) << "Only 'property list uchar int' is supported" << std::endl;
					continue;
				}

				if (name == "vertex_indices")
					header.IndElem = facePropCounter - 1;
			} else {
				PR_LOG(L_ERROR) << "Only float or list properties allowed" << std::endl;
				return;
			}
		} else if (action == "end_header")
			break;
	}

	// Content
	if (!header.hasVertices() || !header.hasIndices() || header.VertexCount <= 0 || header.FaceCount <= 0) {
		PR_LOG(L_WARNING) << "Ply file does not contain valid mesh data" << std::endl;
		return;
	}

	header.SwitchEndianness		  = (method == "binary_big_endian");
	std::unique_ptr<MeshBase> cnt = read(stream, header, (method == "ascii"));

	if (cnt) {
		cnt->triangulate();
		bool useCache = ctx.Env->cache()->shouldCacheMesh(cnt->nodeCount(), static_cast<CacheMode>(mCacheMode));
		ctx.Env->addMesh(mName, std::make_shared<TriMesh>(mName, std::move(cnt), ctx.Env->cache(), useCache));
	}
}
} // namespace PR
