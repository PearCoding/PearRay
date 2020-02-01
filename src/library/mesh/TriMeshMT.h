#pragma once

#include "Mesh.h"

namespace PR {
class PR_LIB TriMeshMT : public Mesh {
public:
	TriMeshMT(const std::string& name,
			std::unique_ptr<MeshBase>&& mesh_base,
			const std::shared_ptr<Cache>& cache,
			bool useCache);
	virtual ~TriMeshMT();

protected:
	void checkCollisionLocal(const RayPackage& in, CollisionOutput& out) override;
	void checkCollisionLocal(const Ray& in, SingleCollisionOutput& out) override;
};
} // namespace PR
