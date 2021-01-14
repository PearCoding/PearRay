#include "WavefrontLoader.h"
#include "Environment.h"
#include "Logger.h"
#include "Platform.h"
#include "SceneLoadContext.h"
#include "mesh/MeshBase.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace PR {
WavefrontLoader::WavefrontLoader(const std::string& override_name)
	: mName(override_name)
	, mScale(1)
	, mFlipNormal(false)
{
}

WavefrontLoader::~WavefrontLoader()
{
}

void WavefrontLoader::load(const std::filesystem::path& file, SceneLoadContext& ctx)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials; // We ignore materials for now
	std::string warn;
	std::string err;

	std::ifstream stream;
	stream.open(file.c_str(), std::ios::in);
	if (!stream) {
		PR_LOG(L_ERROR) << "Wavefront file: Could not open file " << file << std::endl;
		return;
	}

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &stream, nullptr /*material reader*/, true /*triangulate*/, false, true)) {
		PR_LOG(L_ERROR) << "Error reading Wavefront file: " << err << std::endl;
	} else {
		stream.close();

		if (!warn.empty()) {
			PR_LOG(L_WARNING) << "Wavefront file: " << warn << std::endl;
		}

		if (!err.empty()) {
			PR_LOG(L_ERROR) << "Wavefront file: " << err << std::endl;
		}

		if (shapes.empty()) {
			PR_LOG(L_ERROR) << "Wavefront file: No shape given!" << std::endl;
			return;
		}

		if (shapes.size() > 1) {
			PR_LOG(L_WARNING) << "Wavefront file: Contains multiple shapes. Will be combined to one " << std::endl;
		}

		if (attrib.vertices.empty()) {
			PR_LOG(L_ERROR) << "Wavefront file: No vertices given!" << std::endl;
			return;
		}

		// Check if vertex indices and normal indices are the same
		bool hasDiffNormalIndices = false;
		for (size_t sh = 0; sh < shapes.size(); ++sh) {
			for (const auto& index : shapes[sh].mesh.indices) {
				if (index.vertex_index != index.normal_index) {
					hasDiffNormalIndices = true;
					goto hasDiffCheckDone;
				}
			}
		}
	hasDiffCheckDone:

		// Check if vertex indices and texture coordinate indices are the same
		bool hasDiffTextureIndices = false;
		for (size_t sh = 0; sh < shapes.size(); ++sh) {
			for (const auto& index : shapes[sh].mesh.indices) {
				if (index.vertex_index != index.texcoord_index) {
					hasDiffTextureIndices = true;
					goto hasDiffCheckDone2;
				}
			}
		}
	hasDiffCheckDone2:

		// Scale if necessary
		if (mScale != 1.0f) {
			for (size_t i = 0; i < attrib.vertices.size(); ++i)
				attrib.vertices[i] *= mScale;
		}

		// Flip normals if necessary
		if (mFlipNormal) {
			for (size_t i = 0; i < attrib.normals.size(); ++i)
				attrib.normals[i] *= -1;
		}

		// Calculate general (vertex) information
		const size_t vertex_count = attrib.vertices.size();
		size_t indices_count	  = 0;
		for (size_t sh = 0; sh < shapes.size(); ++sh)
			indices_count += shapes[sh].mesh.indices.size();

		size_t face_count = 0;
		for (size_t sh = 0; sh < shapes.size(); ++sh)
			face_count += shapes[sh].mesh.num_face_vertices.size();

		// Setup vertex indices
		std::vector<uint32> indices;
		indices.reserve(indices_count);
		for (size_t sh = 0; sh < shapes.size(); ++sh) {
			const auto& m = shapes[sh].mesh;
			for (size_t i = 0; i < m.indices.size(); ++i)
				indices.push_back(std::max(0, m.indices.at(i).vertex_index));
		}

		auto mesh = std::make_unique<MeshBase>();
		mesh->setVertexComponent(MeshComponent::Vertex, std::move(attrib.vertices));
		mesh->setVertexComponentIndices(MeshComponent::Vertex, std::move(indices));

		// Calculate face vertex count
		std::vector<uint8> faceVertexCount;
		faceVertexCount.resize(face_count);
		size_t offset = 0;
		for (size_t sh = 0; sh < shapes.size(); ++sh) {
			const size_t k = shapes[sh].mesh.num_face_vertices.size();
			std::move(shapes[sh].mesh.num_face_vertices.begin(), shapes[sh].mesh.num_face_vertices.end(), faceVertexCount.begin() + offset);
			offset += k;
		}

		mesh->setFaceVertexCount(std::move(faceVertexCount));

		// Add normals if available
		if (!attrib.normals.empty()) {
			mesh->setVertexComponent(MeshComponent::Normal, std::move(attrib.normals));

			if (hasDiffNormalIndices) {
				std::vector<uint32> norm_indices;
				norm_indices.reserve(indices_count);
				for (size_t sh = 0; sh < shapes.size(); ++sh) {
					const auto& m = shapes[sh].mesh;
					for (size_t i = 0; i < m.indices.size(); ++i)
						norm_indices.push_back(std::max(0, m.indices.at(i).normal_index));
				}
				mesh->setVertexComponentIndices(MeshComponent::Normal, std::move(norm_indices));
			}
		}

		// Add texture coordinates if available
		if (!attrib.texcoords.empty()) {
			mesh->setVertexComponent(MeshComponent::Texture, std::move(attrib.texcoords));

			if (hasDiffTextureIndices) {
				std::vector<uint32> tex_indices;
				tex_indices.reserve(indices_count);
				for (size_t sh = 0; sh < shapes.size(); ++sh) {
					const auto& m = shapes[sh].mesh;
					for (size_t i = 0; i < m.indices.size(); ++i)
						tex_indices.push_back(std::max(0, m.indices.at(i).texcoord_index));
				}
				mesh->setVertexComponentIndices(MeshComponent::Texture, std::move(tex_indices));
			}
		}

		// Add weights if available
		if (attrib.vertex_weights.size() == vertex_count)
			mesh->setVertexComponent(MeshComponent::Weight, std::move(attrib.vertex_weights));

		// Material IDs if available
		// TODO

		// Give mesh a name (default is first shape name)
		std::string name = shapes[0].name;
		if (!mName.empty())
			name = mName;

		if (ctx.hasMesh(name))
			PR_LOG(L_ERROR) << "Mesh " << name << " already in use." << std::endl;

		std::string errMsg;
		if (!mesh->isValid(&errMsg)) {
			PR_LOG(L_WARNING) << "Obj file could not construct a valid mesh data: " << errMsg << std::endl;
			return;
		}

		PR_LOG(L_INFO) << "Added mesh '" << name << "' with " << mesh->triangleCount() << " triangles and " << mesh->quadCount() << " quads" << std::endl;
		ctx.addMesh(name, std::move(mesh));
	}
}
} // namespace PR
