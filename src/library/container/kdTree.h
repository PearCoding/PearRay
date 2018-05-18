#pragma once

#include "geometry/BoundingBox.h"
#include "ray/Ray.h"
#include "shader/FacePoint.h"

#include <algorithm>
#include <iterator>
#include <list>
#include <vector>

#if 0 //def PR_DEBUG
#define PR_KDTREE_DEBUG
#endif

#ifndef PR_KDTREE_MAX_STACK
#define PR_KDTREE_MAX_STACK (4096)
#endif

#ifndef PR_KDTREE_MAX_DEPTH
#define PR_KDTREE_MAX_DEPTH (512)
#endif

/*
 kdTree implementation based on:
 "On building fast kd-Trees for Ray Tracing, and on doing that in O(N log N)"
 by
   Ingo Wald
   Vlastimil Havran
*/
namespace PR {
template <class T, class Observer, bool elementWise = false>
class PR_LIB_INLINE kdTree {
private:
	typedef std::remove_reference_t<std::remove_cv_t<T>> entity_t;
	typedef std::conditional_t<std::is_fundamental<entity_t>::value, entity_t, std::add_lvalue_reference_t<entity_t>> reference_t;
	typedef std::conditional_t<std::is_fundamental<entity_t>::value, reference_t, std::add_lvalue_reference_t<std::add_const_t<entity_t>>> const_reference_t;

	enum SplitPlane {
		SP_Left = 0,
		SP_Right
	};

	enum Side {
		S_Left = 0,
		S_Right,
		S_Both
	};

	struct kdNode {
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		kdNode(uint32 id, bool leaf, const BoundingBox& b)
			: id(id)
			, isLeaf(leaf)
			, boundingBox(b)
		{
		}

		const uint32 id;
		const bool isLeaf;
		BoundingBox boundingBox;
	};

	struct kdInnerNode : public kdNode {
		kdInnerNode(uint32 id, kdNode* l, kdNode* r, const BoundingBox& b)
			: kdNode(id, false, b)
			, left(l)
			, right(r)
		{
		}

		kdNode* left;
		kdNode* right;
	};

	struct kdLeafNode : public kdNode {
		kdLeafNode(uint32 id, const BoundingBox& b)
			: kdNode(id, true, b)
		{
		}

		std::vector<entity_t> objects;
	};

	struct Primitive {
		Primitive(const_reference_t d, const BoundingBox& box)
			: data(d)
			, side(S_Both)
			, box(box)
		{
			//PR_ASSERT(data, "Primitive needs a valid data entry");
		}

		entity_t data;
		Side side;
		BoundingBox box;
	};

public:
	typedef BoundingBox (*GetBoundingBoxCallback)(Observer*, const_reference_t);
	typedef float (*CostCallback)(Observer*, const_reference_t);
	typedef void (*AddedCallback)(Observer*, const_reference_t, uint32 id);

	//typedef bool (*IgnoreCallback)(Observer*, const_reference_t);
	//typedef bool (*CheckCollisionCallback)(Observer*, const Ray&, FacePoint&, const_reference_t);

	inline kdTree(Observer* observer,
				  GetBoundingBoxCallback getBoundingBox,
				  CostCallback cost,
				  AddedCallback addedCallback = nullptr)
		: mRoot(nullptr)
		, mObserver(observer)
		, mNodeCount(0)
		, mGetBoundingBox(getBoundingBox)
		, mCost(cost)
		, mAddedCallback(addedCallback)
		, mDepth(0)
		, mAvgElementsPerLeaf(0)
		, mMinElementsPerLeaf(0)
		, mMaxElementsPerLeaf(0)
		, mLeafCount(0)
		, mMinimumObjectsForLeaf(1)
		, mUseCostForLeaf(true)
	{
		PR_ASSERT(getBoundingBox, "Callback getBoundingBox has to be valid");
		PR_ASSERT(cost, "Callback cost has to be valid");
	}

	virtual ~kdTree()
	{
		deleteNode(mRoot);
	}

	inline uint32 depth() const
	{
		return mDepth;
	}

	inline float avgElementsPerLeaf() const
	{
		return mAvgElementsPerLeaf;
	}

	inline size_t minElementsPerLeaf() const
	{
		return mMinElementsPerLeaf;
	}

	inline size_t maxElementsPerLeaf() const
	{
		return mMaxElementsPerLeaf;
	}

	inline size_t leafCount() const
	{
		return mLeafCount;
	}

	inline size_t minimumObjectsForLeaf() const
	{
		return mMinimumObjectsForLeaf;
	}

	inline void setMinimumObjectsForLeaf(size_t v)
	{
		mMinimumObjectsForLeaf = std::max<size_t>(1, v);
	}

	inline bool useCostForLeafDetection() const
	{
		return mUseCostForLeaf;
	}

	inline void enableCostForLeafDetection(bool b)
	{
		mUseCostForLeaf = b;
	}

	inline bool isEmpty() const
	{
		return mRoot == nullptr || mDepth == 0;
	}

	inline const BoundingBox& boundingBox() const
	{
		PR_ASSERT(mRoot, "Root has to be set before accessing it");
		return mRoot->boundingBox;
	}

	template <typename Iterable>
	inline void build(const Iterable& start, const Iterable& end, size_t size)
	{
		mDepth	 = 0;
		mNodeCount = 0;
		if (start == end)
			return;

		if (size == 1) {
			entity_t e = *start;

			auto leaf = new kdLeafNode(mNodeCount, mGetBoundingBox(mObserver, e));
			leaf->objects.push_back(e);

			mDepth = 1;
			mRoot  = leaf;
			return;
		}

		std::vector<Primitive*> primitives;		// Will be cleared to save memory
		std::vector<Primitive*> primitivesCopy; // Copy to be deleted later

		BoundingBox V;
		for (auto it = start; it != end; ++it) {
			auto prim = new Primitive(*it, mGetBoundingBox(mObserver, *it));
			primitives.push_back(prim);
			primitivesCopy.push_back(prim);
			V.combine(prim->box);
		}

		std::vector<Event> events;
		events.reserve(size * 2);
		for (auto obj : primitives)
			generateEvents(obj, V, events);
		std::sort(events.begin(), events.end());

		mRoot = build(events, primitives, V, 0, -1, 0);

		for (auto obj : primitivesCopy)
			delete obj;

		// Extract statistical information
		mAvgElementsPerLeaf = 0;
		mMinElementsPerLeaf = std::numeric_limits<size_t>::max();
		mMaxElementsPerLeaf = 0;
		mLeafCount			= 0;
		if (mRoot) {
			size_t sum = 0;
			statElementsNode(mRoot, mLeafCount, sum, mMinElementsPerLeaf, mMaxElementsPerLeaf);
			mAvgElementsPerLeaf = sum / (float)mLeafCount;
		}

		if (mMaxElementsPerLeaf < mMinElementsPerLeaf)
			mMinElementsPerLeaf = mMaxElementsPerLeaf;
	}

	template <typename CheckCollisionCallback>
	inline bool checkCollision(const Ray& ray, entity_t& foundEntity, FacePoint& collisionPoint, CheckCollisionCallback checkCollisionCallback) const
	{
		/*thread_local kdNode* stack[PR_KDTREE_MAX_STACK];

		bool found = false;
		FacePoint tmpCollisionPoint;*/

		float t = std::numeric_limits<float>::infinity();

		PR_ASSERT(mRoot, "No root given for kdTree!");
		return checkCollision_Rec(mRoot, ray, t, foundEntity, collisionPoint, checkCollisionCallback);

		/*uint32 stackPos = 1;
		stack[0]		= mRoot;

		while (stackPos > 0) {
			stackPos--;
			kdNode* node = stack[stackPos];

			const auto in = node->boundingBox.intersects(ray);
			if (!in.Successful || in.T > t) {
				continue;
			}

			if (node->leaf == 1) {
				kdLeafNode* leaf = (kdLeafNode*)node;

				for (const_reference_t entity : leaf->objects) {
					if (checkCollisionCallack(mObserver, ray, tmpCollisionPoint, entity)) {
						const float l = (tmpCollisionPoint.P - ray.origin()).squaredNorm();
						if (l < t) {
							t			   = l;
							found		   = true;
							foundEntity	= entity;
							collisionPoint = tmpCollisionPoint;
						}
					}
				}
			} else {
				kdInnerNode* inner = (kdInnerNode*)node;
				if (inner->left) {
					if (stackPos < PR_KDTREE_MAX_STACK) {
						stack[stackPos] = inner->left;
						stackPos++;
					} else {
						// FIXME:
					}
				}

				if (inner->right) {
					if (stackPos < PR_KDTREE_MAX_STACK) {
						stack[stackPos] = inner->right;
						stackPos++;
					} else {
						// FIXME:
					}
				}
			}
		}

		return found;*/
	}

	// A faster variant for rays detecting the background etc.
	template <typename CheckCollisionCallback>
	inline bool checkCollisionSimple(const Ray& ray, FacePoint& collisionPoint, CheckCollisionCallback checkCollisionCallback) const
	{
		//thread_local kdNode* stack[PR_KDTREE_MAX_STACK];

		PR_ASSERT(mRoot, "No root given for kdTree!");

		return checkCollisionSimple_Rec(mRoot, ray, collisionPoint, checkCollisionCallback);
		/*uint32 stackPos = 1;
		stack[0]		= mRoot;

		while (stackPos > 0) {
			stackPos--;
			kdNode* node = stack[stackPos];

			if (!node->boundingBox.intersectsSimple(ray)) {
				continue;
			}

			if (node->leaf == 1) {
				kdLeafNode* leaf = (kdLeafNode*)node;

				for (const_reference_t entity : leaf->objects) {
					if (checkCollisionCallack(mObserver, ray, collisionPoint, entity))
						return true;
				}
			} else {
				kdInnerNode* inner = (kdInnerNode*)node;
				if (inner->left) {
					if (stackPos < PR_KDTREE_MAX_STACK) {
						stack[stackPos] = inner->left;
						stackPos++;
					} else {
						// FIXME:
					}
				}

				if (inner->right) {
					if (stackPos < PR_KDTREE_MAX_STACK) {
						stack[stackPos] = inner->right;
						stackPos++;
					} else {
						// FIXME:
					}
				}
			}
		}

		return false;*/
	}

private:
	template <typename CheckCollisionCallback>
	inline bool checkCollision_Rec(kdNode* node,
								   const Ray& ray, float& t, entity_t& foundEntity, FacePoint& collisionPoint,
								   CheckCollisionCallback checkCollisionCallback) const
	{
		const auto in = node->boundingBox.intersects(ray);
		if (!in.Successful
			|| (!in.Inside && (in.T * in.T) > t)) {
				// We ignore this node when the origin is not inside the block and the distance of intersection is far away
			return false;
		}

		if (node->isLeaf) {
			const kdLeafNode* leaf = (kdLeafNode*)node;

			bool found = false;
			FacePoint tmpCollisionPoint;
			for (const_reference_t entity : leaf->objects) {
				if (checkCollisionCallback(mObserver, ray, tmpCollisionPoint, entity)) {
					const float l = (tmpCollisionPoint.P - ray.origin()).squaredNorm();
					if (l < t) {
						t			   = l;
						found		   = true;
						foundEntity	= entity;
						collisionPoint = tmpCollisionPoint;
					}
				}
			}
			return found;
		} else {
			const kdInnerNode* inner = (kdInnerNode*)node;

			const bool b1 = (inner->left)
					  && checkCollision_Rec(inner->left, ray,
											t, foundEntity, collisionPoint,
											checkCollisionCallback);
			const bool b2 = (inner->right)
					  && checkCollision_Rec(inner->right, ray,
											t, foundEntity, collisionPoint,
											checkCollisionCallback);

			// Do not inline the above calls, as we have to make sure both functions are called!
			return b1 || b2;
		}
	}

	template <typename CheckCollisionCallback>
	inline bool checkCollisionSimple_Rec(kdNode* node,
										 const Ray& ray, FacePoint& collisionPoint,
										 CheckCollisionCallback checkCollisionCallback) const
	{
		if (!node->boundingBox.intersectsSimple(ray)) {
			return false;
		}

		if (node->isLeaf) {
			kdLeafNode* leaf = (kdLeafNode*)node;

			for (const_reference_t entity : leaf->objects) {
				if (checkCollisionCallback(mObserver, ray, collisionPoint, entity))
					return true;
			}
			return false;
		} else {
			kdInnerNode* inner = (kdInnerNode*)node;
			return ((inner->left)
					&& checkCollisionSimple_Rec(inner->left, ray, collisionPoint, checkCollisionCallback))
				   || ((inner->right)
					   && checkCollisionSimple_Rec(inner->right, ray, collisionPoint, checkCollisionCallback));
		}
	}

	static inline void statElementsNode(kdNode* node,
										size_t& leafCount, size_t& sumV, size_t& minV, size_t& maxV)
	{
		if (node->isLeaf) {
			kdLeafNode* leaf = (kdLeafNode*)node;
			++leafCount;

			maxV = std::max(maxV, leaf->objects.size());
			minV = std::min(minV, leaf->objects.size());
			sumV += leaf->objects.size();
		} else {
			kdInnerNode* inner = (kdInnerNode*)node;
			if (inner->left) {
				statElementsNode(inner->left, leafCount, sumV, minV, maxV);
			}

			if (inner->right) {
				statElementsNode(inner->right, leafCount, sumV, minV, maxV);
			}
		}
	}
	static inline void deleteNode(kdNode* node)
	{
		if (node) {
			if (node->isLeaf)
				delete (kdLeafNode*)node;
			else {
				deleteNode(((kdInnerNode*)node)->left);
				deleteNode(((kdInnerNode*)node)->right);
				delete (kdInnerNode*)node;
			}
		}
	}

	static inline float probability(const BoundingBox& VI, const BoundingBox& V)
	{
		PR_ASSERT(V.surfaceArea() > PR_EPSILON, "Surface area should be greater than zero");
		return VI.surfaceArea() / V.surfaceArea();
	}

	static inline void splitBox(const BoundingBox& V, int dim, float v, BoundingBox& VL, BoundingBox& VR)
	{
		VL = V;
		VR = V;

		VL.upperBound()(dim) = v;
		VR.lowerBound()(dim) = v;
	}

	static inline void clipBox(BoundingBox& VI, const BoundingBox& V)
	{
		for (int i = 0; i < 3; i++) {
			if (VI.lowerBound()(i) < V.lowerBound()(i))
				VI.lowerBound()(i) = V.lowerBound()(i);

			if (VI.upperBound()(i) > V.upperBound()(i))
				VI.upperBound()(i) = V.upperBound()(i);
		}
	}

	inline void SAH(float costIntersection, const BoundingBox& V,
					int dim, float v, size_t nl, size_t nr, size_t np,
					float& c, SplitPlane& side, BoundingBox& vl, BoundingBox& vr)
	{
		c = std::numeric_limits<float>::infinity();

		if (V.surfaceArea() <= PR_EPSILON)
			return;

		splitBox(V, dim, v, vl, vr);
		float pl = probability(vl, V);
		float pr = probability(vr, V);

		/*if (pl <= PR_EPSILON || pr <= PR_EPSILON)
			return;*/

		float cl = cost(costIntersection, pl, pr, nl + np, nr);
		float cr = cost(costIntersection, pl, pr, nl, nr + np);

		if (cl < cr) {
			c	= cl;
			side = SP_Left;
		} else {
			c	= cr;
			side = SP_Right;
		}
	}

	inline float cost(float costIntersection, float PL, float PR, size_t NL, size_t NR) const
	{
		return ((NL == 0 || NR == 0) ? 0.8f : 1)
			   * (CostTraversal + costIntersection * (PL * NL + PR * NR));
	}

	enum EventType {
		ET_EndOnPlane   = 0,
		ET_OnPlane		= 1,
		ET_StartOnPlane = 2
	};

	struct Event {
		Primitive* primitive;
		int dim;
		float v;
		EventType type;

		Event()
			: primitive()
			, dim(0)
			, v(0)
			, type(ET_EndOnPlane)
		{
		}

		Event(Primitive* prim, int dim, float v, EventType t)
			: primitive(prim)
			, dim(dim)
			, v(v)
			, type(t)
		{
		}

		inline bool operator<(const Event& other) const
		{
			return ((v < other.v) || (v == other.v && type < other.type));
		}
	};

	inline void generateEvents(Primitive* p, const BoundingBox& V, std::vector<Event>& events)
	{
		BoundingBox box = p->box;
		clipBox(box, V);
		if (box.isPlanar()) {
			for (int i = 0; i < 3; ++i)
				events.push_back(Event(p, i, box.lowerBound()(i), ET_OnPlane));
		} else {
			for (int i = 0; i < 3; ++i)
				events.push_back(Event(p, i, box.upperBound()(i), ET_StartOnPlane));
			for (int i = 0; i < 3; ++i)
				events.push_back(Event(p, i, box.lowerBound()(i), ET_EndOnPlane));
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

		for (size_t j = 0; j < events.size(); ++j) {
			float v				= events[j].v;
			int dim				= events[j].dim;
			size_t onPlane		= 0;
			size_t startOnPlane = 0;
			size_t endOnPlane   = 0;

			while (j < events.size() && events[j].dim == dim
				   && std::abs(events[j].v - v) <= PR_EPSILON
				   && events[j].type == ET_EndOnPlane) {
				++endOnPlane;
				++j;
			}

			while (j < events.size() && events[j].dim == dim
				   && std::abs(events[j].v - v) <= PR_EPSILON
				   && events[j].type == ET_OnPlane) {
				++onPlane;
				++j;
			}

			while (j < events.size() && events[j].dim == dim
				   && std::abs(events[j].v - v) <= PR_EPSILON
				   && events[j].type == ET_StartOnPlane) {
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

			if (c < c_out) {
				c_out	= c;
				dim_out  = dim;
				v_out	= v;
				side_out = side;
				vl_out   = vl;
				vr_out   = vr;
			}

			nl[dim] += startOnPlane + onPlane;
			np[dim] = 0;
		}
	}

	inline void distributeObjects(const std::vector<Primitive*>& objs, int dim, float v, SplitPlane side,
								  std::vector<Primitive*>& left, std::vector<Primitive*>& right)
	{
		for (auto obj : objs) {
			BoundingBox box = obj->box;
			float low		= box.lowerBound()(dim);
			float up		= box.upperBound()(dim);
			if (std::abs(low - v) <= PR_EPSILON
				&& std::abs(up - v) <= PR_EPSILON) {
				if (side == SP_Left)
					left.push_back(obj);
				else
					right.push_back(obj);
			} else {
				if (low < v)
					left.push_back(obj);

				if (up > v)
					right.push_back(obj);
			}
		}
	}

	inline void classify(const std::vector<Event>& events, const std::vector<Primitive*>& objs,
						 int dim, float v, SplitPlane side)
	{
		for (auto obj : objs) {
			obj->side = S_Both;
		}

		for (const Event& e : events) {
			if (e.type == ET_EndOnPlane && e.dim == dim && e.v <= v)
				e.primitive->side = S_Left;
			else if (e.type == ET_StartOnPlane && e.dim == dim && e.v >= v)
				e.primitive->side = S_Right;
			else if (e.type == ET_OnPlane && e.dim == dim) {
				if (e.v < v || (std::abs(e.v - v) <= PR_EPSILON && side == SP_Left))
					e.primitive->side = S_Left;
				if (e.v > v || (std::abs(e.v - v) <= PR_EPSILON && side == SP_Right))
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
		if (!elementWise) {
			costIntersection = mCost(mObserver, entity_t());
		} else {
			costIntersection = 0;
			for (auto obj : objs) {
				costIntersection += mCost(mObserver, obj->data);
			}
			costIntersection /= objs.size();
		}

		int dim			= 0;
		float v			= 0;
		float c			= 0;
		SplitPlane side = SP_Left;
		BoundingBox vl, vr;

		findSplit(costIntersection, events, objs, V, dim, v, c, side, vl, vr);

		if ((/*mUseCostForLeaf &&*/ c > objs.size() * costIntersection)
			//|| objs.size() <= mMinimumObjectsForLeaf
			|| depth > PR_KDTREE_MAX_DEPTH
			|| (prev_dim == dim && std::abs(prev_v - v) <= PR_EPSILON)) {
			auto leaf = new kdLeafNode(mNodeCount++, V);
			for (auto obj : objs) {
				leaf->objects.push_back(obj->data);
				if (mAddedCallback)
					mAddedCallback(mObserver, obj->data, leaf->id);
			}
			return leaf;
		}

		classify(events, objs, dim, v, side);

		// Splice E int ELO and ERO,
		// and generate events for ELB and ERB
		std::vector<Event> leftOnlyEvents, rightOnlyEvents;
		std::vector<Event> leftBothEvents, rightBothEvents;
		for (const Event& e : events) {
			if (e.primitive->side == S_Left)
				leftOnlyEvents.push_back(e);
			else if (e.primitive->side == S_Right)
				rightOnlyEvents.push_back(e);
			else {
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
		std::vector<Event>().swap(leftOnlyEvents);
		std::vector<Event>().swap(leftBothEvents);
		std::vector<Event>().swap(rightOnlyEvents);
		std::vector<Event>().swap(rightBothEvents);
		std::vector<Event>().swap(events);
		std::vector<Primitive*>().swap(objs);

		return new kdInnerNode(mNodeCount++,
							   build(leftEvents, left, vl, depth + 1, dim, v),
							   build(rightEvents, right, vr, depth + 1, dim, v),
							   V);
	}

private:
	float CostTraversal = 1;

	kdNode* mRoot;

	Observer* mObserver;

	size_t mNodeCount;
	GetBoundingBoxCallback mGetBoundingBox;
	CostCallback mCost;
	AddedCallback mAddedCallback;

	uint32 mDepth;
	float mAvgElementsPerLeaf;
	size_t mMinElementsPerLeaf;
	size_t mMaxElementsPerLeaf;
	size_t mLeafCount;

	size_t mMinimumObjectsForLeaf;
	bool mUseCostForLeaf;
};
} // namespace PR
