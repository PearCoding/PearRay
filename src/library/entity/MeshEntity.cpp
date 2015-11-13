#include "MeshEntity.h"

namespace PR
{
	MeshEntity::MeshEntity(const std::string& name, Entity* parent) :
		GeometryEntity(name, parent)
	{
	}

	MeshEntity::~MeshEntity()
	{
	}

	std::string MeshEntity::type() const
	{
		return "mesh";
	}

	void MeshEntity::setMesh(Mesh* mesh)
	{
		mMesh = mesh;
	}

	Mesh* MeshEntity::mesh() const
	{
		return mMesh;
	}

	void MeshEntity::setMaterial(Material* m)
	{
		mMaterial = m;
	}

	Material* MeshEntity::material() const
	{
		return mMaterial;
	}
}