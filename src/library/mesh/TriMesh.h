#pragma once

#include "Mesh.h"

namespace PR {
class PR_LIB TriMesh {
public:
	static std::shared_ptr<Mesh> create(const std::string& name,
										std::unique_ptr<MeshBase>&& mesh_base,
										const std::shared_ptr<Cache>& cache,
										bool useCache);
};
} // namespace PR