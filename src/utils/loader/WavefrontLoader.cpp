#include "WavefrontLoader.h"
#include "geometry/Face.h"
#include "mesh/TriMesh.h"

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

		std::vector<uint32> indices[3];
		std::vector<float> vertices[3];
		std::vector<float> normals[3];
		std::vector<float> uvs[2];

		if (attrib.vertices.empty()) {
			PR_LOG(L_ERROR) << "Wavefront file: No vertices given!" << std::endl;
			return;
		} else {
			vertices[0].reserve(attrib.vertices.size() / 3);
			vertices[1].reserve(attrib.vertices.size() / 3);
			vertices[2].reserve(attrib.vertices.size() / 3);

			for (size_t i = 0; i < attrib.vertices.size() / 3; ++i)
				for (size_t k = 0; k < 3; ++k)
					vertices[k].push_back(attrib.vertices[3 * i + k]);
		}

		if (attrib.normals.size() != attrib.vertices.size()) {
			PR_LOG(L_ERROR) << "Wavefront file: No valid normals given!" << std::endl;
			return;
		} else {
			const float inv = mFlipNormal ? -1.0f : 1.0f;
			normals[0].reserve(attrib.normals.size() / 3);
			normals[1].reserve(attrib.normals.size() / 3);
			normals[2].reserve(attrib.normals.size() / 3);

			for (size_t i = 0; i < attrib.normals.size() / 3; ++i)
				for (size_t k = 0; k < 3; ++k)
					normals[k].push_back(inv * attrib.normals[3 * i + k]);
		}

		if (attrib.texcoords.size() / 2 == vertices[0].size()) {
			uvs[0].reserve(attrib.texcoords.size() / 2);
			uvs[1].reserve(attrib.texcoords.size() / 2);

			for (size_t i = 0; i < attrib.texcoords.size() / 2; ++i)
				for (size_t k = 0; k < 2; ++k)
					uvs[k].push_back(attrib.texcoords[2 * i + k]);
		}

		if (shapes.size() > 1) {
			PR_LOG(L_WARNING) << "Wavefront file: Only one shape per file supported. Ignoring others!" << std::endl;
		}

		if (shapes.empty()) {
			PR_LOG(L_ERROR) << "Wavefront file: No shape given!" << std::endl;
			return;
		}

		const tinyobj::shape_t& shape = shapes[0];
		indices[0].reserve(shape.mesh.indices.size() / 3);
		indices[1].reserve(shape.mesh.indices.size() / 3);
		indices[2].reserve(shape.mesh.indices.size() / 3);

		// TODO: Only support vertex indices!
		for (size_t i = 0; i < shape.mesh.indices.size() / 3; ++i)
			for (size_t k = 0; k < 3; ++k)
				indices[k].push_back(shape.mesh.indices[3 * i + k].vertex_index);

		auto mesh = std::make_shared<TriMesh>();
		mesh->setVertices(vertices[0], vertices[1], vertices[2]);
		mesh->setNormals(normals[0], normals[1], normals[2]);
		mesh->setUVs(uvs[0], uvs[1]);
		mesh->setIndices(indices[0], indices[1], indices[2]);

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
