#pragma once

#include "ray/Ray.h"
#include "shader/FaceSample.h"
#include "geometry/BoundingBox.h"
#include "Logger.h"

#include <list>
#include <vector>
#include <algorithm>
#include <iterator>

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

		enum Side
		{
			S_Left = 0,
			S_Right,
			S_Both
		};

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
			kdLeafNode(const BoundingBox& b) :
				kdNode(1, b)
			{
			}

			std::list<T*> objects;
		};

		struct Primitive
		{
			Primitive(T* data, const BoundingBox& box) :
				data(data), side(S_Both), box(box)
			{
				PR_ASSERT(data);
			}

			T* data;
			Side side;
			BoundingBox box;
		};
	public:
		typedef BoundingBox (*GetBoundingBoxCallback)(T*);
		typedef bool (*CheckCollisionCallback)(const Ray&, FaceSample&, float&, T*);
		typedef float (*CostCallback)(T*);
		typedef bool (*IgnoreCallback)(T*);

		inline kdTree(GetBoundingBoxCallback getBoundingBox,
			CheckCollisionCallback checkCollision,
			CostCallback cost) :
			mRoot(nullptr), mGetBoundingBox(getBoundingBox), mCheckCollision(checkCollision), mCost(cost), mDepth(0)
		{
			PR_ASSERT(getBoundingBox);
			PR_ASSERT(checkCollision);
			PR_ASSERT(cost);
		}

		virtual ~kdTree()
		{
			deleteNode(mRoot);
		}

		inline uint32 depth() const
		{
			return mDepth;
		}

		inline bool isEmpty() const
		{
			return mRoot == nullptr || mDepth == 0;
		}

		inline const BoundingBox& boundingBox() const
		{
			PR_ASSERT(mRoot);
			return mRoot->boundingBox;
		}

		inline void build(const std::list<T*>& entities)
		{
			mDepth = 0;
			if (entities.empty())
				return;

			std::vector<Primitive*> primitives;// Will be cleared to save memory
			std::vector<Primitive*> primitivesCopy;// Copy for delete later

			BoundingBox V;
			for (T* obj : entities)
			{
				auto prim = new Primitive(obj, mGetBoundingBox(obj));
				primitives.push_back(prim);
				primitivesCopy.push_back(prim);
				V.combine(prim->box);
			}

			PR_LOGGER.log(L_Info, M_Scene, "Building kdTree...");

			std::vector<Event> events;
			events.reserve(entities.size() * 2);
			for (auto obj : primitives)
				generateEvents(obj, V, events);
			std::sort(events.begin(), events.end());

			mRoot = build(events, primitives, V, 0, -1, 0);
			PR_LOGGER.logf(L_Info, M_Scene, "-> KD-tree has %d depth.", mDepth);
			if(isEmpty())
				PR_LOGGER.log(L_Warning, M_Scene, "-> KD-tree is empty!");

			for (auto obj : primitivesCopy)
				delete obj;
		}

		inline T* checkCollision(const Ray& ray, FaceSample& collisionPoint, float& t, IgnoreCallback ignoreCallback = nullptr) const {
			PM::vec3 collisionPos;

			T* res = nullptr;
			FaceSample tmpCollisionPoint;

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
							if ((!ignoreCallback || ignoreCallback(entity)) &&
								mCheckCollision(ray, tmpCollisionPoint, l, entity) &&
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
			Primitive* primitive;
			int dim;
			float v;
			EventType type;

			Event() :
				primitive(),dim(0), v(0), type(ET_EndOnPlane)
			{
			}

			Event(Primitive* prim, int dim, float v, EventType t) :
				primitive(prim), dim(dim), v(v), type(t)
			{
			}

			inline bool operator<(const Event& other) const
			{
				return ((v < other.v) ||
					(v == other.v && type < other.type));
			}
		};

		inline void generateEvents(Primitive* p, const BoundingBox& V, std::vector<Event>& events)
		{
			BoundingBox box = p->box;
			clipBox(box, V);
			if (box.isPlanar())
			{
				events.push_back(Event(p, 0, PM::pm_GetIndex(box.lowerBound(), 0), ET_OnPlane));
				events.push_back(Event(p, 1, PM::pm_GetIndex(box.lowerBound(), 1), ET_OnPlane));
				events.push_back(Event(p, 2, PM::pm_GetIndex(box.lowerBound(), 2), ET_OnPlane));
			}
			else
			{
				events.push_back(Event(p, 0, PM::pm_GetIndex(box.lowerBound(), 0), ET_StartOnPlane));
				events.push_back(Event(p, 1, PM::pm_GetIndex(box.lowerBound(), 1), ET_StartOnPlane));
				events.push_back(Event(p, 2, PM::pm_GetIndex(box.lowerBound(), 2), ET_StartOnPlane));
				events.push_back(Event(p, 0, PM::pm_GetIndex(box.upperBound(), 0), ET_EndOnPlane));
				events.push_back(Event(p, 1, PM::pm_GetIndex(box.upperBound(), 1), ET_EndOnPlane));
				events.push_back(Event(p, 2, PM::pm_GetIndex(box.upperBound(), 2), ET_EndOnPlane));
			}
		}

		inline void findSplit(float costIntersection, const std::vector<Event>& events,
			const std::vector<Primitive*>& objs, const BoundingBox& V,
			int& dim_out, float& v_out, float& c_out, SplitPlane& side_out,
			BoundingBox& vl_out, BoundingBox& vr_out)
		{
			c_out = std::numeric_limits<float>::infinity();

			size_t nl[3] = { 0, 0, 0 };
			size_t nr[3] = { objs.size(), objs.size(), objs.size() };
			size_t np[3] = { 0, 0, 0 };

			for (size_t j = 0; j < events.size(); ++j)
			{
				float v = events[j].v;
				int dim = events[j].dim;
				size_t onPlane = 0;
				size_t startOnPlane = 0;
				size_t endOnPlane = 0;

				while (j < events.size() && events[j].dim == dim &&
					std::abs(events[j].v - v) <= PM_EPSILON && events[j].type == ET_EndOnPlane)
				{
					++endOnPlane;
					++j;
				}

				while (j < events.size() && events[j].dim == dim &&
					std::abs(events[j].v - v) <= PM_EPSILON && events[j].type == ET_OnPlane)
				{
					++onPlane;
					++j;
				}

				while (j < events.size() && events[j].dim == dim &&
					std::abs(events[j].v - v) <= PM_EPSILON && events[j].type == ET_StartOnPlane)
				{
					++startOnPlane;
					++j;
				}

				np[dim] = onPlane;
				nr[dim] -= onPlane + endOnPlane;

				float c;
				SplitPlane side;
				BoundingBox vl;
				BoundingBox vr;
				SAH(costIntersection, V, dim, v, nl[dim], nr[dim], np[dim], c, side, vl, vr);

				if (c < c_out)
				{
					c_out = c;
					dim_out = dim;
					v_out = v;
					side_out = side;
					vl_out = vl;
					vr_out = vr;
				}

				nl[dim] += startOnPlane + onPlane;
				np[dim] = 0;
			}
		}

		inline void distributeObjects(const std::vector<Primitive*>& objs, int dim, float v, SplitPlane side,
			std::vector<Primitive*>& left, std::vector<Primitive*>& right)
		{
			for (auto obj : objs)
			{
				BoundingBox box = obj->box;
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

		inline void classify(const std::vector<Event>& events, const std::vector<Primitive*>& objs,
			int dim, float v, SplitPlane side)
		{
			for (auto obj : objs)
			{
				obj->side = S_Both;
			}

			for (const Event& e : events)
			{
				if (e.type == ET_EndOnPlane && e.dim == dim && e.v <= v)
					e.primitive->side = S_Left;
				else if (e.type == ET_StartOnPlane && e.dim == dim && e.v >= v)
					e.primitive->side = S_Right;
				else if (e.type == ET_OnPlane && e.dim == dim)
				{
					if (e.v < v || (std::abs(e.v - v) <= PM_EPSILON && side == SP_Left))
						e.primitive->side = S_Left;
					if (e.v > v || (std::abs(e.v - v) <= PM_EPSILON && side == SP_Right))
						e.primitive->side = S_Right;
				}
			}
		}

		inline kdNode* build(std::vector<Event>& events, std::vector<Primitive*>& objs,
			const BoundingBox& V, uint32 depth,
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
				for (auto obj : objs)
				{
					costIntersection += mCost(obj->data);
				}
				costIntersection /= objs.size();
			}

			int dim;
			float v;
			float c;
			SplitPlane side;
			BoundingBox vl, vr;

			findSplit(costIntersection, events, objs, V, dim, v, c, side, vl, vr);

#ifdef PR_KDTREE_DEBUG
			PR_LOGGER.logf(L_Debug, M_Scene, "%d: N=%d, V=%f, Dim=%d, val=%f, C=%f, Side=%s, VL=%f, VR=%f",
				depth, objs.size(), V.volume(), dim, v, c, side == SP_Left ? "L" : "R", vl.volume(), vr.volume());
#endif

			if (c > objs.size()*costIntersection ||
				depth > PR_KDTREE_MAX_DEPTH ||
				(prev_dim == dim && std::abs(prev_v - v) <= PM_EPSILON))
			{
				auto leaf = new kdLeafNode(V);
				for (auto obj : objs)
				{
					leaf->objects.push_back(obj->data);
				}
				return leaf;
			}

			classify(events, objs, dim, v, side);

			// Splice E int ELO and ERO,
			// and generate events for ELB and ERB
			std::vector<Event> leftOnlyEvents, rightOnlyEvents;
			std::vector<Event> leftBothEvents, rightBothEvents;
			for (const Event& e : events)
			{
				if (e.primitive->side == S_Left)
					leftOnlyEvents.push_back(e);
				else if (e.primitive->side == S_Right)
					rightOnlyEvents.push_back(e);
				else
				{
					generateEvents(e.primitive, vl, leftBothEvents);
					generateEvents(e.primitive, vr, rightBothEvents);
				}
			}

			// Sort 'both' lists ~ O(sqrt(n))
			std::sort(leftBothEvents.begin(), leftBothEvents.end());
			std::sort(rightBothEvents.begin(), rightBothEvents.end());

			// Merge O(n)
			std::vector<Event> leftEvents, rightEvents;
			std::merge(leftOnlyEvents.begin(), leftOnlyEvents.end(), leftBothEvents.begin(), leftBothEvents.end(), std::back_inserter(leftEvents));
			std::merge(rightOnlyEvents.begin(), rightOnlyEvents.end(), rightBothEvents.begin(), rightBothEvents.end(), std::back_inserter(rightEvents));

			// Distribute
			std::vector<Primitive*> left, right;
			distributeObjects(objs, dim, v, side, left, right);

			// Cleanup for memory
			std::vector<Event>().swap(leftOnlyEvents); std::vector<Event>().swap(leftBothEvents);
			std::vector<Event>().swap(rightOnlyEvents); std::vector<Event>().swap(rightBothEvents);
			std::vector<Event>().swap(events);
			std::vector<Primitive*>().swap(objs);

			return new kdInnerNode(
				build(leftEvents, left, vl, depth + 1, dim, v),
				build(rightEvents, right, vr, depth + 1, dim, v),
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