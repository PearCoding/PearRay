#pragma once

#include "Mesh.h"

namespace PR {
class PR_LIB TriMeshPI : public Mesh {
public:
	TriMeshPI(const std::string& name,
			  std::unique_ptr<MeshBase>&& mesh_base,
			  const std::shared_ptr<Cache>& cache,
			  bool useCache);
	virtual ~TriMeshPI();

protected:
	void checkCollisionLocal(const RayPackage& in, CollisionOutput& out) override;
	void checkCollisionLocal(const Ray& in, SingleCollisionOutput& out) override;

private:
	void setup();
	std::unique_ptr<class TriMeshPIInternal> mInternal;
};
} // namespace PR
