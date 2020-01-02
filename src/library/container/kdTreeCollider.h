#pragma once

#include "geometry/BoundingBox.h"
#include "geometry/CollisionData.h"
#include "math/SIMD.h"
#include "ray/RayPackage.h"

#include <algorithm>
#include <iterator>
#include <list>
#include <vector>

#ifndef PR_KDTREE_MAX_STACK
#define PR_KDTREE_MAX_STACK (4096)
#endif

namespace PR {
class Serializer;
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

	// Package Version
	// No coherent optimization
	template <typename CheckCollisionCallback>
	inline bool checkCollision(const RayPackage& in, CollisionOutput& out,
							   CheckCollisionCallback checkCollisionCallback) const
	{
		using namespace simdpp;
		out.HitDistance		   = make_float(std::numeric_limits<float>::infinity());
		const vfloat zero	  = make_zero();
		const Vector3fv invDir = in.Direction.cwiseInverse();

		struct PR_SIMD_ALIGN _stackdata {
			kdNodeCollider* node;
			vfloat minT;
			vfloat maxT;
		};
		thread_local _stackdata stack[PR_KDTREE_MAX_STACK];

		PR_ASSERT(mRoot, "No root given for kdTree!");
		const auto root_in = mBoundingBox.intersectsRange(in);
		if (!any(root_in.Successful))
			return false;

		CollisionOutput tmp;
		size_t stackPos		 = 0;
		stack[stackPos].node = mRoot;
		stack[stackPos].minT = root_in.Entry;
		stack[stackPos].maxT = root_in.Exit;
		++stackPos;

		while (stackPos > 0) {
			kdNodeCollider* currentN = stack[stackPos - 1].node;
			vfloat minT				 = stack[stackPos - 1].minT;
			vfloat maxT				 = stack[stackPos - 1].maxT;
			--stackPos;

			while (!currentN->isLeaf) {
				kdInnerNodeCollider* innerN = reinterpret_cast<kdInnerNodeCollider*>(currentN);
				const vfloat splitM			= innerN->splitPos - in.Origin[innerN->axis];
				const vfloat t				= splitM * invDir[innerN->axis];

				const bfloat dirMask = invDir[innerN->axis] < 0;
				const bfloat eqMask = (in.Direction[innerN->axis]) <= PR_EPSILON;
				const bfloat minHit  = minT <= t;
				const bfloat maxHit  = maxT >= t;

				const bfloat valid = minT <= maxT;
				const bfloat hit   = blend(minHit, maxHit, dirMask);

				const bfloat hitRight = valid & hit | eqMask;
				const bfloat hitLeft  = valid & ~hit | eqMask;

				if (!any(hitRight)) {
					currentN = innerN->left;
				} else if (!any(hitLeft)) {
					currentN = innerN->right;
				} else {
					const uint32 negCount = countNegativeValues(splitM);

					if (negCount <= 2) {
						stack[stackPos].node = innerN->right;
						stack[stackPos].minT = t;
						stack[stackPos].maxT = maxT;
						++stackPos;

						currentN = innerN->left;
						maxT	 = t;
					} else {
						stack[stackPos].node = innerN->left;
						stack[stackPos].minT = minT;
						stack[stackPos].maxT = t;
						++stackPos;

						currentN = innerN->right;
						minT	 = t;
					}
				}

				PR_ASSERT(currentN, "Null node not expected!");
			}

			// Traverse leaf
			PR_ASSERT(currentN->isLeaf, "Expected leaf node!");

			kdLeafNodeCollider* leafN = reinterpret_cast<kdLeafNodeCollider*>(currentN);

			for (uint64 entity : leafN->objects) {
				tmp.HitDistance = make_float(std::numeric_limits<float>::infinity());

				checkCollisionCallback(in, entity, tmp);
				const bfloat hits = (tmp.HitDistance < out.HitDistance)
									& (tmp.HitDistance > zero);

				out.blendFrom(tmp, hits);
			}

			// Early termination
			if (all(out.HitDistance < minT))
				return true;
		}

		return any(out.HitDistance < std::numeric_limits<float>::infinity());
	}

	// Single version
	template <typename CheckCollisionCallback>
	inline bool checkCollision(const Ray& in, SingleCollisionOutput& out,
							   CheckCollisionCallback checkCollisionCallback) const
	{
		using namespace simdpp;
		out.HitDistance		  = std::numeric_limits<float>::infinity();
		const Vector3f invDir = in.Direction.cwiseInverse();

		struct _stackdata {
			kdNodeCollider* node;
			float minT;
			float maxT;
		};
		thread_local _stackdata stack[PR_KDTREE_MAX_STACK];

		PR_ASSERT(mRoot, "No root given for kdTree!");
		const auto root_in = mBoundingBox.intersectsRange(in);
		if (!root_in.Successful)
			return false;

		SingleCollisionOutput tmp;
		size_t stackPos		 = 0;
		stack[stackPos].node = mRoot;
		stack[stackPos].minT = root_in.Entry;
		stack[stackPos].maxT = root_in.Exit;
		++stackPos;

		while (stackPos > 0) {
			kdNodeCollider* currentN = stack[stackPos - 1].node;
			float minT				 = stack[stackPos - 1].minT;
			float maxT				 = stack[stackPos - 1].maxT;
			--stackPos;

			while (!currentN->isLeaf) {
				kdInnerNodeCollider* innerN = reinterpret_cast<kdInnerNodeCollider*>(currentN);
				const float t				= (innerN->splitPos - in.Origin[innerN->axis])
								* invDir[innerN->axis];

				const bool side = (innerN->splitPos > in.Origin[innerN->axis]);

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

			for (uint64 entity : leafN->objects) {
				tmp.HitDistance = std::numeric_limits<float>::infinity();

				// Check for < 0?
				checkCollisionCallback(in, entity, tmp);
				if (tmp.HitDistance < out.HitDistance)
					out = tmp;
			}

			// Early termination
			if (out.HitDistance < minT)
				return true;
		}

		return out.HitDistance < std::numeric_limits<float>::infinity();
	}

	void load(Serializer& stream);

private:
	kdNodeCollider* mRoot;
	size_t mNodeCount;
	BoundingBox mBoundingBox;
};
} // namespace PR
