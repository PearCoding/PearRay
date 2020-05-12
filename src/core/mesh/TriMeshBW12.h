#pragma once

#include "Mesh.h"

namespace PR {
class PR_LIB_CORE TriMeshBW12 : public Mesh {
public:
	TriMeshBW12(const std::string& name,
			  std::unique_ptr<MeshBase>&& mesh_base,
			  const std::shared_ptr<Cache>& cache,
			  bool useCache);
	virtual ~TriMeshBW12();

protected:
	void checkCollisionLocal(const RayPackage& in, CollisionOutput& out) override;
	void checkCollisionLocal(const Ray& in, SingleCollisionOutput& out) override;

private:
	void setup();
	std::unique_ptr<struct TriMeshBW12Internal> mInternal;
};
} // namespace PR
