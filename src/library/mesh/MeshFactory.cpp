#include "MeshFactory.h"
#include "TriMeshMT.h"
#include "TriMeshPI.h"
#include "TriMeshWT.h"
#include "config/TriangleOptions.h"

namespace PR {
std::shared_ptr<Mesh> MeshFactory::create(const std::string& name,
									  std::unique_ptr<MeshBase>&& mesh_base,
									  const std::shared_ptr<Cache>& cache,
									  bool useCache)
{
#if PR_TRIANGLE_INTERSECTION_METHOD == 0
	return std::make_shared<TriMeshMT>(name, std::move(mesh_base), cache, useCache);
#elif PR_TRIANGLE_INTERSECTION_METHOD == 1
	return std::make_shared<TriMeshPI>(name, std::move(mesh_base), cache, useCache);
#elif PR_TRIANGLE_INTERSECTION_METHOD == 2
	return std::make_shared<TriMeshWT>(name, std::move(mesh_base), cache, useCache);
#else
#error Invalid triangle intersection method
	return nullptr;
#endif
}

} // namespace PR
