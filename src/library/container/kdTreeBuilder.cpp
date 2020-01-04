#include "container/kdTreeBuilder.h"
#include "serialization/Serializer.h"
#include <vector>

namespace PR {
enum SplitPlane {
	SP_Left = 0,
	SP_Right
};

enum Side {
	S_Left = 0,
	S_Right,
	S_Both
};

using NodeID = uint32;

struct kdNodeBuilder {
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	kdNodeBuilder(NodeID id, bool leaf, const BoundingBox& b)
		: id(id)
		, isLeaf(leaf)
		, boundingBox(b)
	{
	}

	NodeID id;
	uint8 isLeaf;
	BoundingBox boundingBox;
};

struct kdInnerNodeBuilder : public kdNodeBuilder {
	kdInnerNodeBuilder(NodeID id, uint8 axis, float sp,
					   kdNodeBuilder* l, kdNodeBuilder* r, const BoundingBox& b)
		: kdNodeBuilder(id, false, b)
		, axis(axis)
		, splitPos(sp)
		, left(l)
		, right(r)
	{
	}

	uint8 axis;
	float splitPos;
	kdNodeBuilder* left;
	kdNodeBuilder* right;
};

struct kdLeafNodeBuilder : public kdNodeBuilder {
	kdLeafNodeBuilder(NodeID id, const BoundingBox& b)
		: kdNodeBuilder(id, true, b)
	{
	}

	std::vector<NodeID> objects;
};

struct Primitive {
	Primitive(NodeID d, const BoundingBox& box)
		: data(d)
		, side(S_Both)
		, box(box)
	{
		//PR_ASSERT(data, "Primitive needs a valid data entry");
	}

	NodeID data;
	Side side;
	BoundingBox box;
};

kdTreeBuilder::kdTreeBuilder(void* observer,
							 GetBoundingBoxCallback getBoundingBox,
							 CostCallback cost,
							 AddedCallback addedCallback)
	: mRoot(nullptr)
	, mObserver(observer)
	, mNodeCount(0)
	, mGetBoundingBox(getBoundingBox)
	, mCostCallback(cost)
	, mAddedCallback(addedCallback)
	, mElementWise(false)
	, mDepth(0)
	, mAvgElementsPerLeaf(0)
	, mMinElementsPerLeaf(0)
	, mMaxElementsPerLeaf(0)
	, mLeafCount(0)
{
	PR_ASSERT(getBoundingBox, "Callback getBoundingBox has to be valid");
	PR_ASSERT(cost, "Callback cost has to be valid");
}

static void deleteNode(kdNodeBuilder* node)
{
	if (node) {
		if (node->isLeaf)
			delete reinterpret_cast<kdLeafNodeBuilder*>(node);
		else {
			deleteNode(reinterpret_cast<kdInnerNodeBuilder*>(node)->left);
			deleteNode(reinterpret_cast<kdInnerNodeBuilder*>(node)->right);
			delete reinterpret_cast<kdInnerNodeBuilder*>(node);
		}
	}
}

kdTreeBuilder::~kdTreeBuilder()
{
	deleteNode(mRoot);
}

const BoundingBox& kdTreeBuilder::boundingBox() const
{
	PR_ASSERT(mRoot, "Root has to be set before accessing it");
	return mRoot->boundingBox;
}

void kdTreeBuilder::statElementsNode(kdNodeBuilder* node, size_t& sumV, float root_volume, uint32 depth)
{
	mDepth   = std::max(depth, mDepth);
	node->id = mNodeCount;
	++mNodeCount;

	if (node->isLeaf) {
		kdLeafNodeBuilder* leaf = reinterpret_cast<kdLeafNodeBuilder*>(node);
		++mLeafCount;

		mMaxElementsPerLeaf = std::max(mMaxElementsPerLeaf, leaf->objects.size());
		mMinElementsPerLeaf = std::min(mMinElementsPerLeaf, leaf->objects.size());
		sumV += leaf->objects.size();

		const float ratio = leaf->boundingBox.surfaceArea() / root_volume;
		mExpectedLeavesVisited += ratio;
		mExpectedObjectsIntersected += leaf->objects.size() * ratio;
	} else {
		kdInnerNodeBuilder* inner = reinterpret_cast<kdInnerNodeBuilder*>(node);
		if (inner->left) {
			statElementsNode(inner->left, sumV, root_volume, depth + 1);
		}

		if (inner->right) {
			statElementsNode(inner->right, sumV, root_volume, depth + 1);
		}

		mExpectedTraversalSteps += inner->boundingBox.surfaceArea() / root_volume;
	}
}

static void cleanupNode(kdNodeBuilder*& node)
{
	if (node->isLeaf) {
		return; // Ignore leafs
	} else {
		kdInnerNodeBuilder* innerN = reinterpret_cast<kdInnerNodeBuilder*>(node);

		PR_ASSERT(innerN->left || innerN->right, "Expected atleast one node to be valid!");
		if (innerN->left && !innerN->right) {
			node = innerN->left;
			delete innerN;
			cleanupNode(node);
		} else if (!innerN->left && innerN->right) {
			node = innerN->right;
			delete innerN;
			cleanupNode(node);
		} else {
			cleanupNode(innerN->left);
			cleanupNode(innerN->right);
		}
	}
}

//////////////////////////////////////

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

static inline float cost(float costIntersection, float PL, float PR, size_t NL, size_t NR)
{
	constexpr float CostTraversal = 1;
	return ((NL == 0 || NR == 0) ? 0.8f : 1)
		   * (CostTraversal + costIntersection * (PL * NL + PR * NR));
}

static inline float SAH(float costIntersection, const BoundingBox& V,
						int dim, float v, size_t nl, size_t nr, size_t np,
						SplitPlane& side, BoundingBox& vl, BoundingBox& vr)
{
	splitBox(V, dim, v, vl, vr);

	// Skip non decreasing splits
	if (vl.edge(dim) <= PR_EPSILON || vr.edge(dim) <= PR_EPSILON)
		return std::numeric_limits<float>::infinity();

	const float pl = probability(vl, V);
	const float pr = probability(vr, V);

	const float cl = cost(costIntersection, pl, pr, nl + np, nr);
	const float cr = cost(costIntersection, pl, pr, nl, nr + np);

	if (cl < cr) {
		side = SP_Left;
		return cl;
	} else {
		side = SP_Right;
		return cr;
	}
}

enum EventType : uint8 {
	ET_EndOnPlane   = 0, // -
	ET_OnPlane		= 1, // |
	ET_StartOnPlane = 2  // +
};

struct Event {
	Primitive* primitive;
	uint8 dim;
	float v;
	EventType type;

	Event()
		: primitive()
		, dim(0)
		, v(0)
		, type(ET_EndOnPlane)
	{
	}

	Event(Primitive* prim, uint8 dim, float v, EventType t)
		: primitive(prim)
		, dim(dim)
		, v(v)
		, type(t)
	{
	}

	inline bool operator<(const Event& other) const
	{
		return ((v < other.v)
				|| (/*std::abs(other.v - v) <= PR_EPSILON*/ v == other.v && type < other.type));
	}
};

static void generateEvents(Primitive* p, const BoundingBox& V, std::vector<Event>& events)
{
	BoundingBox box = p->box;
	box.clipBy(V);
	for (int i = 0; i < 3; ++i) {
		if (box.edge(i) <= PR_EPSILON) {
			events.emplace_back(p, i, box.lowerBound()(i), ET_OnPlane);
		} else {
			events.emplace_back(p, i, box.lowerBound()(i), ET_StartOnPlane);
			events.emplace_back(p, i, box.upperBound()(i), ET_EndOnPlane);
		}
	}
}

static void findSplit(float costIntersection, const std::vector<Event>& events,
					  const std::vector<Primitive*>& objs, const BoundingBox& V,
					  uint8& dim_out, float& v_out, float& c_out, SplitPlane& side_out,
					  BoundingBox& vl_out, BoundingBox& vr_out)
{
	PR_ASSERT(std::is_sorted(events.begin(), events.end()), "Expected sorted events list");

	size_t nl[3] = { 0, 0, 0 };
	size_t np[3] = { 0, 0, 0 };
	size_t nr[3] = { objs.size(), objs.size(), objs.size() };

	for (size_t j = 0; j < events.size();) {
		const float v		= events[j].v;
		const uint8 dim		= events[j].dim;
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
		nr[dim] -= onPlane;
		nr[dim] -= endOnPlane;

		SplitPlane side = SP_Left;
		BoundingBox vl;
		BoundingBox vr;
		const float c = SAH(costIntersection, V, dim, v, nl[dim], nr[dim], np[dim], side, vl, vr);

		if (c < c_out) {
			c_out	= c;
			dim_out  = dim;
			v_out	= v;
			side_out = side;
			vl_out   = vl;
			vr_out   = vr;
		}

		np[dim] = 0;
		nl[dim] += startOnPlane;
		nl[dim] += onPlane;
	}
}

static void distributeObjects(const std::vector<Primitive*>& objs,
							  std::vector<Primitive*>& left, std::vector<Primitive*>& right)
{
	for (auto obj : objs) {
		switch (obj->side) {
		case S_Left:
			left.push_back(obj);
			break;
		case S_Right:
			right.push_back(obj);
			break;
		case S_Both:
			left.push_back(obj);
			right.push_back(obj);
			break;
		}
	}
}

static void classify(const std::vector<Event>& events, const std::vector<Primitive*>& objs,
					 uint8 dim, float v, SplitPlane side)
{
	for (auto& obj : objs) {
		obj->side = S_Both;
	}

#ifndef PR_NO_ASSERTS
	for (const Event& e : events) {
		PR_ASSERT(e.primitive->side == S_Both, "Expected primitive to be S_Both");
	}
#endif

	for (const Event& e : events) {
		if (e.dim != dim)
			continue;

		if (e.type == ET_EndOnPlane && e.v <= v) { // | S---E P       |
			e.primitive->side = S_Left;
		} else if (e.type == ET_StartOnPlane && e.v >= v) { // |       P S---E |
			e.primitive->side = S_Right;
		} else if (e.type == ET_OnPlane) {
			// |       P       | or |       P   S   | or |   S   P       |
			if (e.v < v || (std::abs(e.v - v) <= PR_EPSILON && side == SP_Left))
				e.primitive->side = S_Left;
			else //if (e.v > v || (std::abs(e.v - v) <= PR_EPSILON && side == SP_Right))
				e.primitive->side = S_Right;
		}
	}
}

inline static kdNodeBuilder* createLeafNode(size_t& nodeCount, void* observer,
											kdTreeBuilder::AddedCallback addedCallback,
											std::vector<Primitive*>& objs,
											const BoundingBox& V)
{
	kdLeafNodeBuilder* leaf = new kdLeafNodeBuilder(nodeCount++, V);
	for (auto obj : objs) {
		leaf->objects.push_back(obj->data);
		if (addedCallback)
			addedCallback(observer, obj->data, leaf->id);
	}
	return leaf;
}

static kdNodeBuilder* buildNode(size_t& nodeCount, void* observer,
								kdTreeBuilder::CostCallback costCallback,
								kdTreeBuilder::AddedCallback addedCallback,
								std::vector<Event>& events, std::vector<Primitive*>& objs,
								const BoundingBox& V, uint32 depth, uint32 maxDepth, bool elementWise)
{
	if (objs.empty() || V.surfaceArea() <= PR_EPSILON || depth > maxDepth) {
		// Empty leaf or very tiny volume or max depth.
		// Just give up and make a leaf.
		return createLeafNode(nodeCount, observer, addedCallback, objs, V);
	}

	float costIntersection;
	if (!elementWise) {
		costIntersection = costCallback(observer, 0);
	} else {
		costIntersection = 0;
		for (auto obj : objs) {
			costIntersection += costCallback(observer, obj->data);
		}
		costIntersection /= objs.size();
	}

	uint8 dim		= 0;
	float v			= 0;
	float c			= std::numeric_limits<float>::infinity();
	SplitPlane side = SP_Left;
	BoundingBox vl, vr;

	findSplit(costIntersection, events, objs, V, dim, v, c, side, vl, vr);

	if (c > objs.size() * costIntersection)
		return createLeafNode(nodeCount, observer, addedCallback, objs, V);

	classify(events, objs, dim, v, side);

	// Distribute
	std::vector<Primitive*> left, right;
	distributeObjects(objs, left, right);

	// Splice E into ELO and ERO,
	// and generate events for ELB and ERB
	std::vector<Event> leftOnlyEvents, rightOnlyEvents;
	std::vector<Event> leftBothEvents, rightBothEvents;
	for (const Event& e : events) {
		switch (e.primitive->side) {
		case S_Both:
			break;
		case S_Left:
			leftOnlyEvents.push_back(e);
			break;
		case S_Right:
			rightOnlyEvents.push_back(e);
			break;
		}
	}

	for (const auto& p : objs) {
		if (p->side == S_Both) {
			generateEvents(p, vl, leftBothEvents);
			generateEvents(p, vr, rightBothEvents);
		}
	}

	// Sort 'both' lists ~ O(sqrt(n))
	std::sort(leftBothEvents.begin(), leftBothEvents.end());
	std::sort(rightBothEvents.begin(), rightBothEvents.end());

	// Merge O(n)
	std::vector<Event> leftEvents, rightEvents;
	std::merge(leftOnlyEvents.begin(), leftOnlyEvents.end(), leftBothEvents.begin(), leftBothEvents.end(), std::back_inserter(leftEvents));
	std::merge(rightOnlyEvents.begin(), rightOnlyEvents.end(), rightBothEvents.begin(), rightBothEvents.end(), std::back_inserter(rightEvents));

	// Cleanup for memory
	std::vector<Event>().swap(leftOnlyEvents);
	std::vector<Event>().swap(leftBothEvents);
	std::vector<Event>().swap(rightOnlyEvents);
	std::vector<Event>().swap(rightBothEvents);
	std::vector<Event>().swap(events);
	std::vector<Primitive*>().swap(objs);

	return new kdInnerNodeBuilder(nodeCount++, dim, v,
								  buildNode(nodeCount, observer, costCallback, addedCallback,
											leftEvents, left, vl, depth + 1, maxDepth, elementWise),
								  buildNode(nodeCount, observer, costCallback, addedCallback,
											rightEvents, right, vr, depth + 1, maxDepth, elementWise),
								  V);
}

void kdTreeBuilder::build(size_t size)
{
	mDepth	 = 0;
	mMaxDepth  = 0;
	mNodeCount = 0;
	if (size == 0)
		return;
	else if (size == 1) {
		uint64 e = 0;

		auto leaf = new kdLeafNodeBuilder(mNodeCount, mGetBoundingBox(mObserver, e));
		leaf->objects.push_back(e);

		mMaxDepth  = 1;
		mDepth	 = 1;
		mRoot	  = leaf;
		mNodeCount = 1;
		return;
	}

	mMaxDepth = (uint32)std::ceil(8 + 3 * 1.5 * std::log2(size));

	std::vector<Primitive*> primitives;		// Will be cleared to save memory
	std::vector<Primitive*> primitivesCopy; // Copy to be deleted later

	BoundingBox V;
	for (size_t it = 0; it < size; ++it) {
		auto prim = new Primitive(it, mGetBoundingBox(mObserver, it));
		primitives.push_back(prim);
		primitivesCopy.push_back(prim);
		V.combine(prim->box);
	}

	if (V.isPlanar()) {
		V.inflate();
	}

	std::vector<Event> events;
	events.reserve(size * 2);
	for (auto obj : primitives)
		generateEvents(obj, V, events);
	std::sort(events.begin(), events.end());

	mRoot = buildNode(mNodeCount, mObserver, mCostCallback, mAddedCallback,
					  events, primitives, V, 0,
					  mMaxDepth, mElementWise);

	for (auto obj : primitivesCopy)
		delete obj;

	cleanupNode(mRoot);

	// Extract statistical information
	mAvgElementsPerLeaf = 0;
	mMinElementsPerLeaf = std::numeric_limits<size_t>::max();
	mMaxElementsPerLeaf = 0;
	mLeafCount			= 0;

	mExpectedTraversalSteps		= 0;
	mExpectedLeavesVisited		= 0;
	mExpectedObjectsIntersected = 0;

	mDepth = 0;
	if (mRoot) {
		size_t sum = 0;
		statElementsNode(mRoot, sum, mRoot->boundingBox.surfaceArea(), 1);
		mAvgElementsPerLeaf = sum / (float)mLeafCount;
	}

	if (mMaxElementsPerLeaf < mMinElementsPerLeaf)
		mMinElementsPerLeaf = mMaxElementsPerLeaf;
}

/////////////////////////////////////////////

static void saveNode(Serializer& stream, kdNodeBuilder* node)
{
	stream | node->isLeaf;
	if (node->isLeaf) {
		stream | reinterpret_cast<kdLeafNodeBuilder*>(node)->objects;
	} else {
		kdInnerNodeBuilder* innerN = reinterpret_cast<kdInnerNodeBuilder*>(node);
		stream | innerN->axis
			| innerN->splitPos
			| innerN->left->id
			| innerN->right->id;

		saveNode(stream, innerN->left);
		saveNode(stream, innerN->right);
	}
}

void kdTreeBuilder::save(Serializer& stream) const
{
	PR_ASSERT(!stream.isReadMode(), "Expected stream to be in write mode");
	std::string ident = "pearray_kdtree";
	stream | ident;

	if (!mRoot)
		return;

	stream | mRoot->boundingBox.lowerBound();
	stream | mRoot->boundingBox.upperBound();

	saveNode(stream, mRoot);
}

} // namespace PR
