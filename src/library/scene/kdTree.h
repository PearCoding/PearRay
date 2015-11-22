#pragma once

#include "Config.h"
#include "geometry/BoundingBox.h"

#include <list>

#define PR_KDTREE_MAX_DEPTH (128)

namespace PR
{
	class Entity;
	class FacePoint;
	class RenderEntity;
	class PR_LIB kdTree
	{
	public:
		struct kdNode
		{
			kdNode(kdNode* l, kdNode* r, const std::list<RenderEntity*>& list, const BoundingBox& b) :
				left(l), right(r), splitObjects(list), boundingBox(b)
			{
			}

			kdNode* left;
			kdNode* right;
			std::list<RenderEntity*> splitObjects;
			BoundingBox boundingBox;
		};

		kdTree();
		virtual ~kdTree();

		kdNode* root() const;

		void build(const std::list<RenderEntity*>& entities);

		RenderEntity* checkCollision(const Ray& ray, FacePoint& collisionPoint, Entity* ignore = nullptr) const;
	private:
		static void deleteNode(kdNode* node);
		static kdNode* buildNode(size_t depth, const std::list<RenderEntity*>& entities, size_t retryDepth);
		RenderEntity* checkCollisionAtNode(
			const kdNode* node, const Ray& ray, FacePoint& collisionPoint, float& n, Entity* ignore) const;

		kdNode* mRoot;
	};
}