#include "kdTree.h"
#include "entity/GeometryEntity.h"

#include "Logger.h"

namespace PR
{
	kdTree::kdTree() :
		mRoot(nullptr)
	{
	}

	kdTree::~kdTree()
	{
		deleteNode(mRoot);
	}

	kdTree::kdNode* kdTree::root() const
	{
		return mRoot;
	}

	void kdTree::build(const std::list<GeometryEntity*>& entities)
	{
		PR_LOGGER.log(L_Info, M_Scene, "Building kdTree...");

		mRoot = buildNode(0, entities);
	}

	void kdTree::deleteNode(kdNode* node)
	{
		if (node)
		{
			deleteNode(node->left);
			deleteNode(node->right);

			delete node;
		}
	}

	kdTree::kdNode* kdTree::buildNode(size_t depth, const std::list<GeometryEntity*>& entities)
	{
		const uint8 axis = depth % 3;

		if (entities.empty())
		{
			return nullptr;
		}
		else if (entities.size() == 1)
		{
			PR_LOGGER.logf(L_Debug, M_Scene, "[%d|%d] Leaf", depth, axis);
			GeometryEntity* entity = entities.front();
			return new kdNode(nullptr, nullptr, entity, entity->boundingBox());
		}
		else
		{
			BoundingBox box;
			for (GeometryEntity* entity : entities)
			{
				box.combine(entity->boundingBox());
			}

			// Construct next sides
			std::list<GeometryEntity*> leftList;
			std::list<GeometryEntity*> rightList;
			GeometryEntity* midEntity = nullptr;

			float mid = PM::pm_GetX(box.LowerBound) + box.width() / 2;

			if (axis == 1)
			{
				mid = PM::pm_GetY(box.LowerBound) + box.height() / 2;
			}
			else if (axis == 2)
			{
				mid = PM::pm_GetZ(box.LowerBound) + box.depth() / 2;
			}

			// Find nearest entity to median
			float near = 0;
			for (GeometryEntity* e : entities)
			{
				float dist = std::fabsf(mid - PM::pm_GetX(e->boundingBox().UpperBound));
				float dist2 = std::fabsf(mid - PM::pm_GetX(e->boundingBox().LowerBound));

				if (axis == 1)
				{
					dist = std::fabsf(mid - PM::pm_GetY(e->boundingBox().UpperBound));
					dist2 = std::fabsf(mid - PM::pm_GetY(e->boundingBox().LowerBound));
				}
				else if (axis == 2)
				{
					dist = std::fabsf(mid - PM::pm_GetZ(e->boundingBox().UpperBound));
					dist2 = std::fabsf(mid - PM::pm_GetZ(e->boundingBox().LowerBound));
				}

				if (dist > dist2)
				{
					dist = dist2;
				}

				if (!midEntity || near > dist)
				{
					midEntity = e;
					near = dist;
				}
			}

			// Split entities into two parts.
			// Left part can overlap the right part!
			// This algorithm has a left desired tree.
			for (GeometryEntity* e : entities)
			{
				if (e == midEntity)//Ignore mid entity
				{
					continue;
				}

				float dist = mid - PM::pm_GetX(e->boundingBox().UpperBound);
				float dist2 = mid - PM::pm_GetX(e->boundingBox().LowerBound);

				if (axis == 1)
				{
					dist = mid - PM::pm_GetY(e->boundingBox().UpperBound);
					dist2 = mid - PM::pm_GetY(e->boundingBox().LowerBound);
				}
				else if (axis == 2)
				{
					dist = mid - PM::pm_GetZ(e->boundingBox().UpperBound);
					dist2 = mid - PM::pm_GetZ(e->boundingBox().LowerBound);
				}

				if (dist * dist2 >= 0)//Both are positive, or negative
				{
					if (dist < 0)
					{
						leftList.push_back(e);
					}
					else
					{
						rightList.push_back(e);
					}
				}
				else // A entity which is over stretching to the right side
				{
					leftList.push_back(e);// Ignore it and put it into the left side.
				}
			}		

			PR_LOGGER.logf(L_Debug, M_Scene, "[%d|%d] Volume %f | Near %f | Mid %f", depth, axis, box.volume(), near, mid);
			return new kdNode(buildNode(depth+1, leftList), buildNode(depth+1, rightList), midEntity, box);
		}
	}
}