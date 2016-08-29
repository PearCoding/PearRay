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
		virtual float collisionCost() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, SamplePoint& collisionPoint) override;

		virtual SamplePoint getRandomFacePoint(Sampler& sampler, uint32 sample) const;
	private:
		IMesh* mMesh;
	};
}