#pragma once

#include "RenderEntity.h"

namespace PR
{
	class IMesh;
	class Material;
	class PR_LIB MeshEntity : public RenderEntity
	{
	public:
		MeshEntity(const std::string& name, Entity* parent = nullptr);
		virtual ~MeshEntity();

		virtual std::string type() const;

		virtual bool isLight() const override;

		void setMesh(IMesh* mesh);
		IMesh* mesh() const;

		virtual bool isCollidable() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t) override;

		virtual FacePoint getRandomFacePoint(Sampler& sampler, uint32 sample) const;
	private:
		IMesh* mMesh;
	};
}