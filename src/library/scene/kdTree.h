#pragma once

#include "ray/Ray.h"
#include "geometry/BoundingBox.h"
#include "geometry/FacePoint.h"
#include "Logger.h"

#include <list>
#include <functional>

#ifdef PR_DEBUG
//# define PR_KDTREE_DEBUG
#endif

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
		typedef std::function<bool(const Ray&, FacePoint&, float&, T*, T*)> CheckCollisionCallback;

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

#ifdef PR_KDTREE_DEBUG
					PR_LOGGER.logf(L_Debug, M_Scene, "[%d] Leaf | Volume %f", stackPos, mGetBoundingBox(entity).volume());
#endif

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
					T* midEntity = nullptr;

					float mid = PM::pm_GetIndex(box.center(), axis);

					// Find nearest entity to median
					float near = 0;
					for (T* e : content.Entities)
					{
						BoundingBox bx = mGetBoundingBox(e);

						const float dist = PM::pm_MinT(std::abs(mid - PM::pm_GetIndex(bx.upperBound(), axis)),
							std::abs(mid - PM::pm_GetIndex(bx.lowerBound(), axis)));

						if (!midEntity || near > dist)
						{
							midEntity = e;
							near = dist;
						}
					}

					if (!midEntity)
						continue;//Nothing available

#ifdef PR_KDTREE_DEBUG
					PR_LOGGER.logf(L_Debug, M_Scene, "[%d, %d] Volume %f | Near %f | Mid %f | Side %d", stackPos, axis, box.volume(), near, mid, content.Side);
#endif

					// Split entities into two parts.
					uint8 c = 0;
					for (T* e : content.Entities)
					{
						if (e == midEntity)//Ignore mid entity
							continue;

						BoundingBox bx = mGetBoundingBox(e);
						const float dist = mid - PM::pm_GetIndex(bx.upperBound(), axis);
						const float dist2 = mid - PM::pm_GetIndex(bx.lowerBound(), axis);
						const float midDist = 0.5f* (dist + dist2);

						if (std::abs(midDist) >= PM_EPSILON)//Both are positive, or negative
						{
							if (midDist < 0)
								leftList.push_back(e);
							else
								rightList.push_back(e);
						}
						else // A entity which is split by the plane.
						{
							if (c == 0)
								leftList.push_back(e);
							else
								rightList.push_back(e);

							c = (c + 1) % 2;
						}
					}

					// DO NOT USE content AFTER THIS LINE

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

		inline T* checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t, T* ignore = nullptr) const {
			PM::vec3 collisionPos;

			T* res = nullptr;
			FacePoint tmpCollisionPoint;

			t = std::numeric_limits<float>::max();
			float l = 0;// Temporary variable.
			kdNode* stack[PR_KDTREE_MAX_STACK];
			uint32 stackPos = 1;

			stack[0] = root();

			if (root() && root()->boundingBox.intersects(ray, collisionPos, l))
			{
				while (stackPos > 0)
				{
					stackPos--;
					kdNode* node = stack[stackPos];
					
					if (node->object != ignore &&
						mCheckCollision(ray, tmpCollisionPoint, l, node->object, ignore) &&
						l < t)
					{
						t = l;
						res = node->object;
						collisionPoint = tmpCollisionPoint;
					}

					bool leftIntersect = false;
					if (node->left && node->left->boundingBox.intersects(ray, collisionPos, l))
					{
						if (stackPos >= PR_KDTREE_MAX_STACK)
						{
							return nullptr;
						}

						leftIntersect = true;
						stack[stackPos] = node->left;
						stackPos++;
					}

					if (node->right && (!leftIntersect || node->right->boundingBox.intersects(ray, collisionPos, l)))
					{
						if (stackPos >= PR_KDTREE_MAX_STACK)
						{
							return nullptr;
						}

						stack[stackPos] = node->right;
						stackPos++;
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