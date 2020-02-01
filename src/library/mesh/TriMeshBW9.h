#pragma once

#include "Mesh.h"

namespace PR {
class PR_LIB TriMeshBW9 : public Mesh {
public:
	TriMeshBW9(const std::string& name,
			  std::unique_ptr<MeshBase>&& mesh_base,
			  const std::shared_ptr<Cache>& cache,
			  bool useCache);
	virtual ~TriMeshBW9();

protected:
	void checkCollisionLocal(const RayPackage& in, CollisionOutput& out) override;
	void checkCollisionLocal(const Ray& in, SingleCollisionOutput& out) override;

private:
	void setup();
	std::unique_ptr<class TriMeshBW9Internal> mInternal;
};
} // namespace PR
