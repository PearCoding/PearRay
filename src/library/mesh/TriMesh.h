#pragma once

#include "Mesh.h"

namespace PR {
// Simple collidable mesh
class PR_LIB TriMesh : public Mesh {
public:
	TriMesh(const std::string& name,
			std::unique_ptr<MeshBase>&& mesh_base,
			const std::shared_ptr<Cache>& cache,
			bool useCache);
	virtual ~TriMesh();

protected:
	void checkCollisionLocal(const RayPackage& in, CollisionOutput& out) override;
	void checkCollisionLocal(const Ray& in, SingleCollisionOutput& out) override;

private:
	void setup();
	std::unique_ptr<struct TriMeshInternal> mInternal;
};
} // namespace PR
