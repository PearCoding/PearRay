#pragma once

#include "Config.h"
#include "geometry/BoundingBox.h"

#include <list>

namespace PR
{
	class GeometryEntity;
	class kdTree
	{
	public:
		struct kdNode
		{
			kdNode(kdNode* l, kdNode* r, GeometryEntity* e, const BoundingBox& b) :
				left(l), right(r), entry(e), boundingBox(b)
			{
			}

			kdNode* left;
			kdNode* right;
			GeometryEntity* entry;
			BoundingBox boundingBox;
		};

		kdTree();
		virtual ~kdTree();

		kdNode* root() const;

		void build(const std::list<GeometryEntity*>& entities);
	private:
		static void deleteNode(kdNode* node);
		static kdNode* buildNode(size_t depth, const std::list<GeometryEntity*>& entities);

		kdNode* mRoot;
	};
}