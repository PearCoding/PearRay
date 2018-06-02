#include "WavefrontLoader.h"
#include "geometry/Face.h"
#include "geometry/TriMesh.h"

#include "Environment.h"
#include "Logger.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace PR {
WavefrontLoader::WavefrontLoader(const std::map<std::string, std::string>& overrides)
	: mOverrides(overrides)
	, mScale(1)
	, mFlipNormal(false)
{
}

WavefrontLoader::~WavefrontLoader()
{
}

void WavefrontLoader::load(const std::string& file, Environment* env)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials; // We ignore materials for now
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str())) {
		PR_LOG(L_ERROR) << "Error reading Wavefront file: " << err << std::endl;
	} else {
		if (!err.empty()) {
			PR_LOG(L_WARNING) << "Wavefront file: " << err << std::endl;
		}

		std::vector<Vector3u64> indices;
		std::vector<Vector3f> vertices;
		std::vector<Vector3f> normals;
		std::vector<Vector2f> uvs;

		if (attrib.vertices.empty()) {
			PR_LOG(L_ERROR) << "Wavefront file: No vertices given!" << std::endl;
			return;
		} else {
			vertices.reserve(attrib.vertices.size() / 3);
			for (size_t i = 0; i < attrib.vertices.size() / 3; ++i) {
				vertices.push_back(Vector3f{ attrib.vertices[3 * i + 0], attrib.vertices[3 * i + 1], attrib.vertices[3 * i + 2] });
			}
		}

		if (attrib.normals.size() != attrib.vertices.size()) {
			PR_LOG(L_ERROR) << "Wavefront file: No valid normals given!" << std::endl;
			return;
		} else {
			const float inv = mFlipNormal ? -1.0f : 1.0f;
			normals.reserve(attrib.normals.size() / 3);
			for (size_t i = 0; i < attrib.normals.size() / 3; ++i) {
				normals.push_back(Vector3f{inv * attrib.normals[3 * i + 0], inv * attrib.normals[3 * i + 1], inv * attrib.normals[3 * i + 2] });
			}
		}

		if(attrib.texcoords.size() / 2 == vertices.size()) {
			uvs.reserve(attrib.texcoords.size() / 2);
			for (size_t i = 0; i < attrib.texcoords.size() / 2; ++i) {
				uvs.push_back(Vector2f{ attrib.texcoords[2 * i + 0], attrib.texcoords[2 * i + 1]});
			}
		}

		if(shapes.size() > 1) {
			PR_LOG(L_WARNING) << "Wavefront file: Only one shape per file supported. Ignoring others!" << std::endl;
		}

		if(shapes.empty()) {
			PR_LOG(L_ERROR) << "Wavefront file: No shape given!" << std::endl;
			return;
		}

		const tinyobj::shape_t& shape = shapes[0];
		indices.reserve(shape.mesh.indices.size());

		// TODO: Only support vertex indices!
		for (size_t i = 0; i < shape.mesh.indices.size() / 3; ++i) {
			indices.push_back(Vector3u64{shape.mesh.indices[3 * i + 0].vertex_index, shape.mesh.indices[3 * i + 1].vertex_index, shape.mesh.indices[3 * i + 2].vertex_index});
		}

		auto mesh = std::make_shared<PR::TriMesh>();
		mesh->setVertices(vertices);
		mesh->setNormals(normals);
		mesh->setUVs(uvs);
		mesh->setIndices(indices);

		std::string name = shape.name;
		if (mOverrides.count(shape.name))
			name = mOverrides[shape.name];

		int i			 = 1;
		std::string next = name;
		while (env->hasMesh(next) && i < 1000) {
			std::stringstream stream;
			stream << name << "_" << i;
			next = stream.str();
			i++;
		}

		if (env->hasMesh(next))
			PR_LOG(L_ERROR) << "Mesh " << next << " already in use." << std::endl;

		env->addMesh(next, mesh);
		PR_LOG(L_INFO) << "Added mesh " << next << std::endl;
	}
}
} // namespace PR
