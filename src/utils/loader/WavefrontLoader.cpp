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
		PR_LOGGER.instance().logf(L_Error, M_Scene, "Error reading Wavefront file: %s", err.c_str());
	} else {
		if (!err.empty()) {
			PR_LOGGER.instance().logf(L_Warning, M_Scene, "Wavefront file: %s", err.c_str());
		}

		for (tinyobj::shape_t shape : shapes) {
			auto mesh = std::make_shared<PR::TriMesh>();
			mesh->reserve(shape.mesh.indices.size() / 3);

			float inv = mFlipNormal ? -1.0f : 1.0f;

			for (size_t i = 0; i < shape.mesh.indices.size() / 3; ++i) {
				Face* face = new Face;
				for (int j = 0; j < 3; j++) {
					tinyobj::index_t idx = shape.mesh.indices[3 * i + j];

					face->V[j] = Eigen::Vector3f(mScale * attrib.vertices[3 * idx.vertex_index + 0],
												 mScale * attrib.vertices[3 * idx.vertex_index + 1],
												 mScale * attrib.vertices[3 * idx.vertex_index + 2]);

					face->N[j] = Eigen::Vector3f(inv * attrib.normals[3 * idx.normal_index + 0],
												 inv * attrib.normals[3 * idx.normal_index + 1],
												 inv * attrib.normals[3 * idx.normal_index + 2])
									 .normalized();

					face->UV[j] = Eigen::Vector2f(attrib.texcoords[2 * idx.texcoord_index + 0],
												  attrib.texcoords[2 * idx.texcoord_index + 1]);
				}

				mesh->addFace(face);
			}

			mesh->build();

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
				PR_LOGGER.logf(L_Error, M_Scene, "Mesh '%s' already in use.", next.c_str());

			env->addMesh(next, mesh);
			PR_LOGGER.logf(L_Info, M_Scene, "Added mesh '%s'", next.c_str());
		}
	}
}
} // namespace PR
