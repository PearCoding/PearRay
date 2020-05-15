#pragma once

#include "Mesh.h"

namespace PR {
class PR_LIB_CORE TriMeshPIOff : public Mesh {
public:
	TriMeshPIOff(const std::string& name,
			  std::unique_ptr<MeshBase>&& mesh_base,
			  const std::shared_ptr<Cache>& cache,
			  bool useCache);
	virtual ~TriMeshPIOff();

protected:
	void checkCollisionLocal(const RayPackage& in, CollisionOutput& out) override;
	void checkCollisionLocal(const Ray& in, SingleCollisionOutput& out) override;
};
} // namespace PR