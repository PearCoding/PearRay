#pragma once

#include "geometry/BoundingBox.h"
#include "ray/Ray.h"
#include "shader/FacePoint.h"

#include <algorithm>
#include <iterator>
#include <list>
#include <vector>

#ifndef PR_KDTREE_MAX_STACK
#define PR_KDTREE_MAX_STACK (4096)
#endif

namespace PR {
class PR_LIB kdTreeCollider {
public:
	struct kdNodeCollider {
		kdNodeCollider(uint32 id, bool leaf)
			: id(id)
			, isLeaf(leaf)
		{
		}

		const uint32 id;
		const bool isLeaf;
	};

	struct kdInnerNodeCollider : public kdNodeCollider {
		kdInnerNodeCollider(uint32 id, uint8 axis, float sp,
							kdNodeCollider* l, kdNodeCollider* r)
			: kdNodeCollider(id, false)
			, axis(axis)
			, splitPos(sp)
			, left(l)
			, right(r)
		{
		}

		const uint8 axis;
		const float splitPos;
		kdNodeCollider* left;
		kdNodeCollider* right;
	};

	struct kdLeafNodeCollider : public kdNodeCollider {
		kdLeafNodeCollider(uint32 id)
			: kdNodeCollider(id, true)
		{
		}

		std::vector<uint64> objects;
	};

public:
	kdTreeCollider();
	virtual ~kdTreeCollider();

	inline bool isEmpty() const
	{
		return mRoot == nullptr;
	}

	inline const BoundingBox& boundingBox() const
	{
		return mBoundingBox;
	}

	template <typename CheckCollisionCallback>
	inline bool checkCollision(const Ray& ray, uint64& foundEntity, FacePoint& collisionPoint, CheckCollisionCallback checkCollisionCallback) const
	{
		struct _stackdata {
			kdNodeCollider* node;
			float minT;
			float maxT;
		};
		thread_local _stackdata stack[PR_KDTREE_MAX_STACK];

		PR_ASSERT(mRoot, "No root given for kdTree!");
		const auto root_in = mBoundingBox.intersectsRange(ray);
		if (!root_in.Successful)
			return false;

		float hitDistance   = std::numeric_limits<float>::infinity();
		bool found			= false;
		const Eigen::Vector3f invDir = ray.direction().cwiseInverse();

		size_t stackPos				 = 0;
		stack[stackPos].node		 = mRoot;
		stack[stackPos].minT		 = std::max(0.0f, root_in.Entry);
		stack[stackPos].maxT		 = root_in.Exit;
		++stackPos;

		// Scene behind ray origin
		if (stack[0].minT > stack[0].maxT)
			return false;

		while (stackPos > 0) {
			kdNodeCollider* currentN = stack[stackPos - 1].node;
			float minT				 = stack[stackPos - 1].minT;
			float maxT				 = stack[stackPos - 1].maxT;
			--stackPos;

			while (!currentN->isLeaf) {
				kdInnerNodeCollider* innerN = reinterpret_cast<kdInnerNodeCollider*>(currentN);
				const float t				= (innerN->splitPos - ray.origin()(innerN->axis))
								* invDir(innerN->axis);

				const bool side		  = (innerN->splitPos > ray.origin()(innerN->axis))
				|| (std::abs(innerN->splitPos - ray.origin()(innerN->axis)) <= PR_EPSILON && ray.direction()(innerN->axis) <= 0);
				kdNodeCollider* nearN = side ? innerN->left : innerN->right;
				kdNodeCollider* farN  = side ? innerN->right : innerN->left;

				if (t > maxT || t <= 0) {
					currentN = nearN;
				} else if (t < minT) {
					currentN = farN;
				} else {
					stack[stackPos].node = farN;
					stack[stackPos].minT = t;
					stack[stackPos].maxT = maxT;
					++stackPos;

					currentN = nearN;
					maxT	 = t;
				}

				PR_ASSERT(currentN, "Null node not expected!");
			}

			// Traverse leaf
			PR_ASSERT(currentN->isLeaf, "Expected leaf node!");

			kdLeafNodeCollider* leafN = reinterpret_cast<kdLeafNodeCollider*>(currentN);

			FacePoint tmpCollisionPoint;
			for (uint64 entity : leafN->objects) {
				float l;
				if (checkCollisionCallback(ray, tmpCollisionPoint, entity, l)) {
					if (l < hitDistance) {
						hitDistance	= l;
						found		   = true;
						foundEntity	= entity;
						collisionPoint = tmpCollisionPoint;
					}
				}
			}

			if(hitDistance < minT)
				return true;
		}

		return found;
	}

	// A faster variant for rays detecting the background etc.
	template <typename CheckCollisionCallback>
	inline bool checkCollisionSimple(const Ray& ray, FacePoint& collisionPoint, CheckCollisionCallback checkCollisionCallback) const
	{
		struct _stackdata {
			kdNodeCollider* node;
			float minT;
			float maxT;
		};
		thread_local _stackdata stack[PR_KDTREE_MAX_STACK];

		PR_ASSERT(mRoot, "No root given for kdTree!");
		const auto root_in = mBoundingBox.intersectsRange(ray);
		if (!root_in.Successful)
			return false;

		const Eigen::Vector3f invDir = ray.direction().cwiseInverse();

		int stackPos		 = 0;
		stack[stackPos].node = mRoot;
		stack[stackPos].minT = std::max(0.0f, root_in.Entry);
		stack[stackPos].maxT = root_in.Exit;
		++stackPos;
		
		// Scene behind ray origin
		if (stack[0].minT > stack[0].maxT)
			return false;

		while (stackPos > 0) {
			kdNodeCollider* currentN = stack[stackPos - 1].node;
			float minT				 = stack[stackPos - 1].minT;
			float maxT				 = stack[stackPos - 1].maxT;
			--stackPos;

			while (!currentN->isLeaf) {
				kdInnerNodeCollider* innerN = reinterpret_cast<kdInnerNodeCollider*>(currentN);
				const float t				= (innerN->splitPos - ray.origin()(innerN->axis))
								* invDir(innerN->axis);

				const bool side		  = (innerN->splitPos > ray.origin()(innerN->axis))
				|| (std::abs(innerN->splitPos - ray.origin()(innerN->axis)) <= PR_EPSILON && ray.direction()(innerN->axis) <= 0);
				kdNodeCollider* nearN = side ? innerN->left : innerN->right;
				kdNodeCollider* farN  = side ? innerN->right : innerN->left;

				if (t > maxT || t <= 0) {
					currentN = nearN;
				} else if (t < minT) {
					currentN = farN;
				} else {
					stack[stackPos].node = farN;
					stack[stackPos].minT = t;
					stack[stackPos].maxT = maxT;
					++stackPos;

					currentN = nearN;
					maxT	 = t;
				}

				PR_ASSERT(currentN, "Null node not expected!");
			}

			// Traverse leaf
			PR_ASSERT(currentN->isLeaf, "Expected leaf node!");
			kdLeafNodeCollider* leafN = (kdLeafNodeCollider*)currentN;

			for (uint64 entity : leafN->objects) {
				if (checkCollisionCallback(ray, collisionPoint, entity)) {
					return true;
				}
			}
		}

		return false;
	}

	void load(std::istream& stream);

private:
	kdNodeCollider* mRoot;
	size_t mNodeCount;
	BoundingBox mBoundingBox;
};
} // namespace PR
