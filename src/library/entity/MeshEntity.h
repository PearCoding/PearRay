#pragma once

#include "RenderEntity.h"

namespace PR
{
	class Mesh;
	class Material;
	class PR_LIB MeshEntity : public RenderEntity
	{
	public:
		MeshEntity(const std::string& name, Entity* parent = nullptr);
		virtual ~MeshEntity();

		virtual std::string type() const;

		void setMesh(Mesh* mesh);
		Mesh* mesh() const;

		bool isLight() const;
		void setMaterial(Material* m);
		Material* material() const;

		virtual bool isCollidable() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint) override;

		virtual void apply(Ray& in, const FacePoint& point, Renderer* renderer) override;

		virtual FacePoint getRandomFacePoint(Random& random) const;
	private:
		Mesh* mMesh;
		Material* mMaterial;
	};
}