#pragma once

#include "ray/Ray.h"
#include "geometry/BoundingBox.h"
#include "geometry/FacePoint.h"
#include "Logger.h"

#include <list>
#include <vector>
#include <functional>
#include <algorithm>

#ifdef PR_DEBUG
//# define PR_KDTREE_DEBUG
#endif

#ifndef PR_KDTREE_MAX_STACK
# define PR_KDTREE_MAX_STACK (4096)
#endif

#ifndef PR_KDTREE_MAX_DEPTH
# define PR_KDTREE_MAX_DEPTH (512)
#endif

/*
 kdTree implementation based on:
 "On building fast kd-Trees for Ray Tracing, and on doing that in O(N log N)"
 by
   Ingo Wald
   Vlastimil Havran
*/
namespace PR
{
	template<class T, bool elementWise = false>
	class PR_LIB_INLINE kdTree
	{
	private:
		enum SplitPlane
		{
			SP_Left = 0,
			SP_Right
		};

	public:
		typedef std::function<BoundingBox(T*)> GetBoundingBoxCallback;
		typedef std::function<bool(const Ray&, FacePoint&, float&, T*, T*)> CheckCollisionCallback;
		typedef std::function<float(T*)> CostCallback;

		struct kdNode
		{
			kdNode(uint8 l, const BoundingBox& b) :
				leaf(l), boundingBox(b)
			{
			}

			const uint8 leaf;
			BoundingBox boundingBox;
		};

		struct kdInnerNode : public kdNode
		{
			kdInnerNode(kdNode* l, kdNode* r, const BoundingBox& b) :
				kdNode(0, b), left(l), right(r)
			{
			}

			kdNode* left;
			kdNode* right;
		};

		struct kdLeafNode : public kdNode
		{
			kdLeafNode(const std::list<T*>& obj, const BoundingBox& b) :
				kdNode(1, b), objects(obj)
			{
			}

			std::list<T*> objects;
		};

		inline kdTree(GetBoundingBoxCallback getBoundingBox, CheckCollisionCallback checkCollision, CostCallback cost) :
			mRoot(nullptr), mGetBoundingBox(getBoundingBox), mCheckCollision(checkCollision), mCost(cost), mDepth(0)
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

		inline uint32 depth() const
		{
			return mDepth;
		}

		inline void build(const std::list<T*>& entities)
		{
			mDepth = 0;
			if (entities.empty())
				return;

			BoundingBox box;
			for (T* obj : entities)
			{
				box.combine(mGetBoundingBox(obj));
			}

			PR_LOGGER.log(L_Info, M_Scene, "Building kdTree...");
			mRoot = build(entities, box, 0, -1, 0);
			PR_LOGGER.logf(L_Info, M_Scene, "-> KD-tree has %d depth.", mDepth);
		}

		inline T* checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t, T* ignore = nullptr) const {
			PM::vec3 collisionPos;

			T* res = nullptr;
			FacePoint tmpCollisionPoint;

			t = std::numeric_limits<float>::infinity();
			float l = t;// Temporary variable.

			if (mRoot && mRoot->boundingBox.intersects(ray, collisionPos, l))
			{
				kdNode* stack[PR_KDTREE_MAX_STACK];
				uint32 stackPos = 1;
				stack[0] = mRoot;

				while (stackPos > 0)
				{
					stackPos--;
					kdNode* node = stack[stackPos];
					
					if (node->leaf == 1)
					{
						kdLeafNode* leaf = (kdLeafNode*)node;

						for (T* entity : leaf->objects)
						{
							if (entity != ignore &&
								mCheckCollision(ray, tmpCollisionPoint, l, entity, ignore) &&
								l < t)
							{
								t = l;
								res = entity;
								collisionPoint = tmpCollisionPoint;
							}
						}
					}
					else
					{
						bool leftIntersected = false;
						kdInnerNode* inner = (kdInnerNode*)node;
						if (inner->left && inner->left->boundingBox.intersects(ray, collisionPos, l))
						{
							if (stackPos >= PR_KDTREE_MAX_STACK)
								return nullptr;

							leftIntersected = true;
							stack[stackPos] = inner->left;
							stackPos++;
						}

						if (inner->right &&
							(!leftIntersected || inner->right->boundingBox.intersects(ray, collisionPos, l)))
						{
							if (stackPos >= PR_KDTREE_MAX_STACK)
								return nullptr;

							stack[stackPos] = inner->right;
							stackPos++;
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
				if (node->leaf == 1)
					delete (kdLeafNode*)node;
				else
				{
					deleteNode(((kdInnerNode*)node)->left);
					deleteNode(((kdInnerNode*)node)->right);
					delete (kdInnerNode*)node;
				}
			}
		}

		static inline float probability(const BoundingBox& VI, const BoundingBox& V)
		{
			PR_ASSERT(V.surfaceArea() > PM_EPSILON);
			return VI.surfaceArea() / V.surfaceArea();
		}

		static inline void splitBox(const BoundingBox& V, int dim, float v, BoundingBox& VL, BoundingBox& VR)
		{
			VL = V;
			VR = V;
			VL.setUpperBound(PM::pm_SetIndex(VL.upperBound(), dim, v));
			VR.setLowerBound(PM::pm_SetIndex(VR.lowerBound(), dim, v));
		}

		static inline void clipBox(BoundingBox& VI, const BoundingBox& V)
		{
			for (int i = 0; i < 3; i++)
			{
				if (PM::pm_GetIndex(VI.lowerBound(), i) < PM::pm_GetIndex(V.lowerBound(), i))
					VI.setLowerBound(PM::pm_SetIndex(VI.lowerBound(), i, PM::pm_GetIndex(V.lowerBound(), i)));

				if (PM::pm_GetIndex(VI.upperBound(), i) > PM::pm_GetIndex(V.upperBound(), i))
					VI.setUpperBound(PM::pm_SetIndex(VI.upperBound(), i, PM::pm_GetIndex(V.upperBound(), i)));
			}
		}

		inline void SAH(float costIntersection, const BoundingBox& V, int dim, float v, size_t nl, size_t nr, size_t np,
			float& c, SplitPlane& side, BoundingBox& vl, BoundingBox& vr)
		{
			c = std::numeric_limits<float>::infinity();

			if (V.surfaceArea() <= PM_EPSILON)
				return;

			splitBox(V, dim, v, vl, vr);
			float pl = probability(vl, V);
			float pr = probability(vr, V);

			if (pl <= PM_EPSILON || pr <= PM_EPSILON)
				return;

			float cl = cost(costIntersection, pl, pr, nl + np, nr);
			float cr = cost(costIntersection, pl, pr, nl, nr + np);

			if (cl < cr)
			{
				c = cl;
				side = SP_Left;
			}
			else
			{
				c = cr;
				side = SP_Right;
			}
		}

		/*
		 Normally all primitives/objects have their own 'cost',
		 but we approximate it. Everything else decreases performances.
		 */
		inline float cost(float costIntersection, float PL, float PR, size_t NL, size_t NR) const
		{
			return ((NL == 0 || NR == 0) ? 0.8f : 1)*(CostTraversal + costIntersection*(PL*NL + PR*NR));
		}

		enum EventType
		{
			ET_EndOnPlane = 0,
			ET_OnPlane = 1,
			ET_StartOnPlane = 2
		};

		struct Event
		{
			int dim;
			float v;
			EventType type;

			Event() :
				dim(0), v(0), type(ET_EndOnPlane)
			{
			}

			Event(int dim, float v, EventType t) :
				dim(dim), v(v), type(t)
			{
			}

			inline bool operator<(const Event& other) const
			{
				return ((v < other.v) ||
					(std::abs(v - other.v) <= PM_EPSILON && type < other.type));
			}
		};

		// TODO: Implement O(n) search, without sorting in every dimension
		inline void findSplit(float costIntersection, const std::list<T*>& objs, const BoundingBox& V,
			int& dim_out, float& v_out, float& c_out, SplitPlane& side_out,
			BoundingBox& vl_out, BoundingBox& vr_out)
		{
			c_out = std::numeric_limits<float>::infinity();
			for (int i = 0; i < 3; ++i)
			{
				std::vector<Event> events;
				events.reserve(objs.size() * 2);

				for (T* obj : objs)
				{
					BoundingBox box = mGetBoundingBox(obj);
					clipBox(box, V);
					if (box.isPlanar())
					{
						events.push_back(Event(i, PM::pm_GetIndex(box.lowerBound(), i), ET_OnPlane));
					}
					else
					{
						events.push_back(Event(i, PM::pm_GetIndex(box.lowerBound(), i), ET_StartOnPlane));
						events.push_back(Event(i, PM::pm_GetIndex(box.upperBound(), i), ET_EndOnPlane));
					}
				}

				std::sort(events.begin(), events.end());

				size_t nl = 0;
				size_t nr = objs.size();
				size_t np = 0;

				for (size_t j = 0; j < events.size(); ++j)
				{
					float v = events[j].v;
					size_t onPlane = 0;
					size_t startOnPlane = 0;
					size_t endOnPlane = 0;

					while (j < events.size() && std::abs(events[j].v - v) <= PM_EPSILON && events[j].type == ET_EndOnPlane)
					{
						endOnPlane++;
						j++;
					}

					while (j < events.size() && std::abs(events[j].v - v) <= PM_EPSILON && events[j].type == ET_OnPlane)
					{
						onPlane++;
						j++;
					}

					while (j < events.size() && std::abs(events[j].v - v) <= PM_EPSILON && events[j].type == ET_StartOnPlane)
					{
						startOnPlane++;
						j++;
					}

					np = onPlane;
					nr -= onPlane + endOnPlane;

					float c;
					SplitPlane side;
					BoundingBox vl;
					BoundingBox vr;
					SAH(costIntersection, V, i, v, nl, nr, np, c, side, vl, vr);

					if (c < c_out)
					{
						c_out = c;
						dim_out = i;
						v_out = v;
						side_out = side;
						vl_out = vl;
						vr_out = vr;
					}

					nl += startOnPlane + onPlane;
					np = 0;
				}
			}
		}

		inline void distributeObjects(const std::list<T*>& objs, int dim, float v, SplitPlane side,
			std::list<T*>& left, std::list<T*>& right)
		{
			for (T* obj : objs)
			{
				BoundingBox box = mGetBoundingBox(obj);
				float low = PM::pm_GetIndex(box.lowerBound(), dim);
				float up = PM::pm_GetIndex(box.upperBound(), dim);
				if (std::abs(low - v) <= PM_EPSILON &&
					std::abs(up - v) <= PM_EPSILON)
				{
					if (side == SP_Left)
						left.push_back(obj);
					else
						right.push_back(obj);
				}
				else
				{
					if(low < v)
						left.push_back(obj);
					
					if(up > v)
						right.push_back(obj);
				}
			}
		}

		inline kdNode* build(const std::list<T*>& objs, const BoundingBox& V, uint32 depth,
			int prev_dim, float prev_v)
		{
			if (objs.size() == 0)
				return nullptr;

			if (depth > mDepth)
				mDepth = depth;

			float costIntersection;
			if (!elementWise)
			{
				costIntersection = mCost(nullptr);
			}
			else
			{
				costIntersection = 0;
				for (T* obj : objs)
				{
					costIntersection += mCost(obj);
				}
				costIntersection /= objs.size();
			}

			int dim;
			float v;
			float c;
			SplitPlane side;
			BoundingBox vl, vr;

			findSplit(costIntersection, objs, V, dim, v, c, side, vl, vr);

			PR_LOGGER.logf(L_Debug, M_Scene, "%d: N=%d, V=%f, Dim=%d, val=%f, C=%f, Side=%s, VL=%f, VR=%f",
				depth, objs.size(), V.volume(), dim, v, c, side == SP_Left ? "L" : "R", vl.volume(), vr.volume());

			if (c > objs.size()*costIntersection ||
				depth > PR_KDTREE_MAX_DEPTH ||
				(prev_dim == dim && std::abs(prev_v - v) <= PM_EPSILON))
				return new kdLeafNode(objs, V);

			std::list<T*> left, right;
			distributeObjects(objs, dim, v, side, left, right);
			return new kdInnerNode(
				build(left, vl, depth + 1, dim, v),
				build(right, vr, depth + 1, dim, v),
				V);
		}
	private:
		float CostTraversal = 1;

		kdNode* mRoot;

		GetBoundingBoxCallback mGetBoundingBox;
		CheckCollisionCallback mCheckCollision;
		CostCallback mCost;

		uint32 mDepth;
	};
}