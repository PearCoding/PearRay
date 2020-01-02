#include "WavefrontLoader.h"
#include "Environment.h"
#include "Logger.h"
#include "Platform.h"
#include "SceneLoadContext.h"
#include "cache/Cache.h"
#include "mesh/MeshBase.h"
#include "mesh/TriMesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace PR {
WavefrontLoader::WavefrontLoader(const std::string& override_name)
	: mName(override_name)
	, mScale(1)
	, mFlipNormal(false)
	, mCacheMode(CM_Auto)
{
}

WavefrontLoader::~WavefrontLoader()
{
}

void WavefrontLoader::load(const std::wstring& file, const SceneLoadContext& ctx)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials; // We ignore materials for now
	std::string warn;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
						  std::string(file.begin(), file.end()).c_str())) {
		PR_LOG(L_ERROR) << "Error reading Wavefront file: " << err << std::endl;
	} else {
		if (!warn.empty()) {
			PR_LOG(L_WARNING) << "Wavefront file: " << warn << std::endl;
		}

		if (!err.empty()) {
			PR_LOG(L_ERROR) << "Wavefront file: " << err << std::endl;
		}

		if (attrib.vertices.empty()) {
			PR_LOG(L_ERROR) << "Wavefront file: No vertices given!" << std::endl;
			return;
		}

		bool hasNorms = (attrib.normals.size() == attrib.vertices.size());
		if (hasNorms && mFlipNormal) {
			if (mFlipNormal) {
				for (size_t i = 0; i < attrib.normals.size(); ++i)
					attrib.normals[i] *= -1;
			}
		}

		bool hasCoords = (attrib.texcoords.size() / 2 == attrib.vertices.size() / 3);

		if (shapes.empty()) {
			PR_LOG(L_ERROR) << "Wavefront file: No shape given!" << std::endl;
			return;
		}

		size_t indices_count = 0;
		for (size_t sh = 0; sh < shapes.size(); ++sh)
			indices_count += shapes[sh].mesh.indices.size();

		std::vector<uint32> indices;
		indices.reserve(indices_count);
		for (size_t sh = 0; sh < shapes.size(); ++sh) {
			for (size_t i = 0; i < shapes[sh].mesh.indices.size(); ++i)
				indices.push_back(shapes[sh].mesh.indices.at(i).vertex_index);
		}

		auto mesh = std::make_unique<MeshBase>();
		mesh->setVertices(attrib.vertices);
		mesh->setIndices(indices);
		mesh->setFaceVertexCount(std::vector<uint8>(indices_count / 3, 3));
		if (hasNorms)
			mesh->setNormals(attrib.normals);
		else
			mesh->buildNormals();
		if (hasCoords)
			mesh->setUVs(attrib.texcoords);

		std::string name = shapes[0].name;
		if (!mName.empty())
			name = mName;

		int i			 = 1;
		std::string next = name;
		while (ctx.Env->hasMesh(next) && i < 1000) {
			std::stringstream stream;
			stream << name << "_" << i;
			next = stream.str();
			i++;
		}

		if (ctx.Env->hasMesh(next))
			PR_LOG(L_ERROR) << "Mesh " << next << " already in use." << std::endl;

		mesh->triangulate();
		bool useCache = ctx.Env->cache()->shouldCacheMesh(mesh->nodeCount(), static_cast<CacheMode>(mCacheMode));
		ctx.Env->addMesh(next, std::make_shared<TriMesh>(next, std::move(mesh), ctx.Env->cache(), useCache));
		PR_LOG(L_INFO) << "Added mesh " << next << std::endl;
	}
}
} // namespace PR
