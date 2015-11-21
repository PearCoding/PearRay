#include "kdTree.h"
#include "entity/GeometryEntity.h"
#include "ray/Ray.h"
#include "geometry/BoundingBox.h"
#include "geometry/FacePoint.h"

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

		mRoot = buildNode(0, entities, 0);
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

	kdTree::kdNode* kdTree::buildNode(size_t depth, const std::list<GeometryEntity*>& entities, size_t retryDepth)
	{
		const uint8 axis = depth % 3;

		if (entities.empty() || retryDepth >= 3)
		{
			return nullptr;
		}
		else if (entities.size() == 1)
		{
			GeometryEntity* entity = entities.front();
			PR_LOGGER.logf(L_Debug, M_Scene, "[%d|%d] Leaf | Volume %f", depth, axis, entity->worldBoundingBox().volume());
			PR_LOGGER.logf(L_Debug, M_Scene, "       -> Object: %s", entity->toString().c_str());

			return new kdNode(nullptr, nullptr, entities, entity->worldBoundingBox());
		}
		else
		{
			BoundingBox box;
			for (GeometryEntity* entity : entities)
			{
				box.combine(entity->worldBoundingBox());
			}

			if (depth >= PR_KDTREE_MAX_DEPTH)
			{
				PR_LOGGER.logf(L_Debug, M_Scene, "[%d|%d] Max Depth reached! | Volume %f", depth, axis, box.volume());
				return new kdNode(nullptr, nullptr, entities, box);
			}

			// Construct next sides
			std::list<GeometryEntity*> leftList;
			std::list<GeometryEntity*> rightList;
			std::list<GeometryEntity*> midList;
			GeometryEntity* midEntity = nullptr;

			float mid = PM::pm_GetX(box.center());
			if (axis == 1)
			{
				mid = PM::pm_GetY(box.center());
			}
			else if (axis == 2)
			{
				mid = PM::pm_GetZ(box.center());
			}

			// Find nearest entity to median
			float near = 0;
			for (GeometryEntity* e : entities)
			{
				BoundingBox bx = e->worldBoundingBox();

				float dist = std::fabsf(mid - PM::pm_GetX(bx.upperBound()));
				float dist2 = std::fabsf(mid - PM::pm_GetX(bx.lowerBound()));

				if (axis == 1)
				{
					dist = std::fabsf(mid - PM::pm_GetY(bx.upperBound()));
					dist2 = std::fabsf(mid - PM::pm_GetY(bx.lowerBound()));
				}
				else if (axis == 2)
				{
					dist = std::fabsf(mid - PM::pm_GetZ(bx.upperBound()));
					dist2 = std::fabsf(mid - PM::pm_GetZ(bx.lowerBound()));
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

			PR_LOGGER.logf(L_Debug, M_Scene, "[%d|%d] Volume %f | Near %f | Mid %f", depth, axis, box.volume(), near, mid);
			PR_LOGGER.logf(L_Debug, M_Scene, "       -> Mid Object: %s", midEntity->toString().c_str());
			midList.push_back(midEntity);

			// Split entities into two parts.
			for (GeometryEntity* e : entities)
			{
				if (e == midEntity)//Ignore mid entity
				{
					continue;
				}

				BoundingBox bx = e->worldBoundingBox();
				float dist = mid - PM::pm_GetX(bx.upperBound());
				float dist2 = mid - PM::pm_GetX(bx.lowerBound());

				if (axis == 1)
				{
					dist = mid - PM::pm_GetY(bx.upperBound());
					dist2 = mid - PM::pm_GetY(bx.lowerBound());
				}
				else if (axis == 2)
				{
					dist = mid - PM::pm_GetZ(bx.upperBound());
					dist2 = mid - PM::pm_GetZ(bx.lowerBound());
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
				else // A entity which is split by the plane.
				{
					PR_LOGGER.logf(L_Debug, M_Scene, "       -> Split Object: %s", e->toString().c_str());
					midList.push_back(e);
				}
			}		

			if (midList.size() == entities.size())// Seems this axis is to plane and useless to cut with
			{
				PR_LOGGER.log(L_Debug, M_Scene, "       -> Ignoring Axis.");
				kdNode* node = buildNode(depth + 1, entities, retryDepth+1);

				if (!node)// Retrying did not work, now we have to accept our fate...
				{
					return new kdNode(nullptr, nullptr, midList, box);
				}
				else
				{
					return node;
				}
			}
			else
			{
				return new kdNode(buildNode(depth + 1, leftList, 0), buildNode(depth + 1, rightList, 0), midList, box);
			}
		}
	}

	GeometryEntity* kdTree::checkCollision(const Ray& ray, FacePoint& collisionPoint, Entity* ignore) const
	{
		float n = std::numeric_limits<float>::max();
		return checkCollisionAtNode(root(), ray, collisionPoint, n, ignore);
	}

	GeometryEntity* kdTree::checkCollisionAtNode(
		const kdNode* node, const Ray& ray, FacePoint& collisionPoint, float& near, Entity* ignore) const
	{
		if (node && node->boundingBox.intersects(ray))
		{
			GeometryEntity* res = nullptr;
			FacePoint tmpCollisionPoint;

			// First the mid elements one by one
			for (GeometryEntity* e : node->splitObjects)
			{
				if ((Entity*)e != ignore && e->checkCollision(ray, tmpCollisionPoint))
				{
					float l = PM::pm_Magnitude3D(PM::pm_Subtract(tmpCollisionPoint.vertex(), ray.startPosition()));

					if (l < near)
					{
						near = l;
						res = e;
						collisionPoint = tmpCollisionPoint;
					}
				}
			}

			// Now check left with recursion
			GeometryEntity* left = checkCollisionAtNode(node->left, ray, tmpCollisionPoint, near, ignore);
			if (left)
			{
				res = left;
				collisionPoint = tmpCollisionPoint;
			}

			// And of course check the right one with recursion as well
			GeometryEntity* right = checkCollisionAtNode(node->right, ray, tmpCollisionPoint, near, ignore);
			if (right)
			{
				res = right;
				collisionPoint = tmpCollisionPoint;
			}

			return res;
		}
		else
		{
			return nullptr;
		}
	}
}