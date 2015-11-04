#pragma once

#include "GeometryEntity.h"

namespace PR
{
	class Mesh;
	class Material;
	class MeshEntity : public GeometryEntity
	{
	public:
		MeshEntity(const std::string& name, Entity* parent = nullptr);
		virtual ~MeshEntity();

		void setMesh(Mesh* mesh);
		Mesh* mesh() const;

		void setMaterial(Material* m);
		Material* material() const;

	private:
		Mesh* mMesh;
		Material* mMaterial;
	};
}