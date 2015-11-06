#pragma once

#include "Config.h"
#include "geometry/BoundingBox.h"

#include <list>

#define PR_KDTREE_MAX_DEPTH (128)

namespace PR
{
	class FacePoint;
	class GeometryEntity;
	class PR_LIB kdTree
	{
	public:
		struct kdNode
		{
			kdNode(kdNode* l, kdNode* r, const std::list<GeometryEntity*>& list, const BoundingBox& b) :
				left(l), right(r), splitObjects(list), boundingBox(b)
			{
			}

			kdNode* left;
			kdNode* right;
			std::list<GeometryEntity*> splitObjects;
			BoundingBox boundingBox;
		};

		kdTree();
		virtual ~kdTree();

		kdNode* root() const;

		void build(const std::list<GeometryEntity*>& entities);

		GeometryEntity* checkCollision(const Ray& ray, FacePoint& collisionPoint) const;
	private:
		static void deleteNode(kdNode* node);
		static kdNode* buildNode(size_t depth, const std::list<GeometryEntity*>& entities, size_t retryDepth);
		GeometryEntity* checkCollisionAtNode(const kdNode* node, const Ray& ray, FacePoint& collisionPoint) const;

		kdNode* mRoot;
	};
}