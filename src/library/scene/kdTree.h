#pragma once

#include "ray/Ray.h"
#include "geometry/BoundingBox.h"
#include "geometry/FacePoint.h"
#include "Logger.h"

#include <list>
#include <functional>

#ifndef PR_KDTREE_MAX_STACK
# define PR_KDTREE_MAX_STACK (4096)
#endif

namespace PR
{
	// TODO: Add balancing methods
	template<class T>
	class PR_LIB_INLINE kdTree
	{
	public:
		typedef std::function<BoundingBox(T*)> GetBoundingBoxCallback;
		typedef std::function<bool(const Ray&, FacePoint&, T*, T*)> CheckCollisionCallback;

		struct kdNode
		{
			kdNode(kdNode* l, kdNode* r, T* obj, const BoundingBox& b) :
				left(l), right(r), object(obj), boundingBox(b)
			{
			}

			kdNode* left;
			kdNode* right;
			T* object;
			BoundingBox boundingBox;
		};

		inline kdTree(GetBoundingBoxCallback getBoundingBox, CheckCollisionCallback checkCollision) :
			mRoot(nullptr), mGetBoundingBox(getBoundingBox), mCheckCollision(checkCollision)
		{
		}

		virtual ~kdTree()
		{
			deleteNode(mRoot);
		}

		inline kdNode* root() const
		{
			return mRoot;
		}

		inline void build(const std::list<T*>& entities)
		{
			if (entities.empty())
				return;

			struct StackContent
			{
				std::list<T*> Entities;
				kdNode* Parent;
				uint8 Side;
			};

			PR_LOGGER.log(L_Info, M_Scene, "Building kdTree...");
			StackContent stack[PR_KDTREE_MAX_STACK];
			uint32 stackPos = 0;

			stack[stackPos] = { entities, nullptr, 0 };
			stackPos++;

			while (stackPos > 0)
			{
				stackPos--;
				StackContent& content = stack[stackPos];
				kdNode* parent = content.Parent;
				uint8 side = content.Side;

				if (content.Entities.size() == 1)
				{
					T* entity = content.Entities.front();
					PR_LOGGER.logf(L_Debug, M_Scene, "[%d] Leaf | Volume %f", stackPos, mGetBoundingBox(entity).volume());
					//PR_LOGGER.logf(L_Debug, M_Scene, "       -> Object: %s", entity->toString().c_str());

					auto node = new kdNode(nullptr, nullptr, entity, mGetBoundingBox(entity));
					if (parent)
					{
						if (side == 0)
						{
							parent->left = node;
						}
						else
						{
							parent->right = node;
						}
					}
					else
					{
						mRoot = node;
					}
				}
				else
				{
					BoundingBox box;
					for (T* entity : content.Entities)
					{
						box.combine(mGetBoundingBox(entity));
					}

					int axis = 2;// Z axis
					if (box.width() > box.height() &&
						box.width() > box.depth())
						axis = 0;// X axis
					else if (box.height() > box.depth())
						axis = 1;// Y axis

					// Construct next sides
					std::list<T*> leftList;
					std::list<T*> rightList;
					std::list<T*> midList;
					T* midEntity = nullptr;

					float mid = PM::pm_GetIndex(box.center(), axis);

					// Find nearest entity to median
					float near = 0;
					for (T* e : content.Entities)
					{
						BoundingBox bx = mGetBoundingBox(e);

						float dist = PM::pm_MinT(std::abs(mid - PM::pm_GetIndex(bx.upperBound(), axis)),
							std::abs(mid - PM::pm_GetIndex(bx.lowerBound(), axis)));

						if (!midEntity || near > dist)
						{
							midEntity = e;
							near = dist;
						}
					}

					if (!midEntity)
						continue;//Nothing available

					PR_LOGGER.logf(L_Debug, M_Scene, "[%d, %d] Volume %f | Near %f | Mid %f | Side %d", stackPos, axis, box.volume(), near, mid, content.Side);

					// Split entities into two parts.
					for (T* e : content.Entities)
					{
						if (e == midEntity)//Ignore mid entity
						{
							continue;
						}

						BoundingBox bx = mGetBoundingBox(e);
						float dist = mid - PM::pm_GetIndex(bx.upperBound(), axis);
						float dist2 = mid - PM::pm_GetIndex(bx.lowerBound(), axis);

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
							midList.push_back(e);
						}
					}

					// DO NOT USE content AFTER THIS LINE

					//leftList.insert(leftList.end(), midList.begin(), midList.end());
					//rightList.insert(rightList.end(), midList.begin(), midList.end());
					uint32 c = 0;
					for (T* e : midList)
					{
						if (c % 2 == 0)
							leftList.push_back(e);
						else
							rightList.push_back(e);
						c++;
					}

					auto node = new kdNode(nullptr, nullptr, midEntity, box);
					if (!leftList.empty())
					{
						stack[stackPos] = { leftList, node, 0 };
						stackPos++;
						PR_ASSERT(stackPos < PR_KDTREE_MAX_STACK);
					}

					if (!rightList.empty())
					{
						stack[stackPos] = { rightList, node, 1 };
						stackPos++;
						PR_ASSERT(stackPos < PR_KDTREE_MAX_STACK);
					}

					if (parent)
					{
						if (side == 0)
						{
							parent->left = node;
						}
						else
						{
							parent->right = node;
						}
					}
					else
					{
						mRoot = node;
					}
				}
			}
		}

		inline T* checkCollision(const Ray& ray, FacePoint& collisionPoint, T* ignore = nullptr) const {
			PM::vec3 collisionPos;

			T* res = nullptr;
			FacePoint tmpCollisionPoint;

			float n = std::numeric_limits<float>::max();
			float l = 0;// Temporary variable.
			kdNode* stack[PR_KDTREE_MAX_STACK];
			float near[PR_KDTREE_MAX_STACK];
			uint32 stackPos = 1;

			stack[0] = root();
			near[0] = 0;

			if (root() && root()->boundingBox.intersects(ray, collisionPos))
			{
				while (stackPos > 0)
				{
					stackPos--;
					kdNode* node = stack[stackPos];

					/*if (near[stackPos] > n)
						continue;*/

					if (node->object != ignore &&
						mCheckCollision(ray, tmpCollisionPoint, node->object, ignore))
					{
						l = PM::pm_MagnitudeSqr3D(PM::pm_Subtract(tmpCollisionPoint.vertex(), ray.startPosition()));

						if (l < n)
						{
							n = l;
							res = node->object;
							collisionPoint = tmpCollisionPoint;
						}
					}

					if (node->left && node->left->boundingBox.intersects(ray, collisionPos))
					{
						l = PM::pm_MagnitudeSqr3D(PM::pm_Subtract(collisionPos, ray.startPosition()));
						if (l <= n)
						{
							stack[stackPos] = node->left;
							near[stackPos] = l;
							stackPos++;

							PR_ASSERT(stackPos < PR_KDTREE_MAX_STACK);
						}
					}

					if (node->right && node->right->boundingBox.intersects(ray, collisionPos))
					{
						l = PM::pm_MagnitudeSqr3D(PM::pm_Subtract(collisionPos, ray.startPosition()));
						if (l <= n)
						{
							stack[stackPos] = node->right;
							near[stackPos] = l;
							stackPos++;

							PR_ASSERT(stackPos < PR_KDTREE_MAX_STACK);
						}
					}
				}
			}

			return res;
		}
	private:
		static inline void deleteNode(kdNode* node)
		{
			if (node)
			{
				deleteNode(node->left);
				deleteNode(node->right);

				delete node;
			}
		}

		kdNode* mRoot;

		GetBoundingBoxCallback mGetBoundingBox;
		CheckCollisionCallback mCheckCollision;
	};
}