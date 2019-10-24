#include "container/kdTreeBuilderNaive.h"

#include <algorithm>
#include <vector>

namespace PR {

enum Side {
	S_Left = 0,
	S_Right,
	S_Both,
	S_Planar
};

enum SidePlane {
	SP_Left,
	SP_Right
};

struct kdNodeBuilder {
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	kdNodeBuilder(uint32 id, bool leaf, const BoundingBox& b)
		: id(id)
		, isLeaf(leaf)
		, boundingBox(b)
	{
	}

	uint32 id;
	const bool isLeaf;
	BoundingBox boundingBox;
};

struct kdInnerNodeBuilder : public kdNodeBuilder {
	kdInnerNodeBuilder(uint32 id, uint8 axis, float sp,
					   kdNodeBuilder* l, kdNodeBuilder* r, const BoundingBox& b)
		: kdNodeBuilder(id, false, b)
		, axis(axis)
		, splitPos(sp)
		, left(l)
		, right(r)
	{
	}

	const uint8 axis;
	const float splitPos;
	kdNodeBuilder* left;
	kdNodeBuilder* right;
};

struct kdLeafNodeBuilder : public kdNodeBuilder {
	kdLeafNodeBuilder(uint32 id, const BoundingBox& b)
		: kdNodeBuilder(id, true, b)
	{
	}

	std::vector<uint64> objects;
};

struct Primitive {
	Primitive(uint64 d, const BoundingBox& box)
		: data(d)
		, side(S_Both)
		, box(box)
	{
	}

	uint64 data;
	Side side;
	BoundingBox box;
};

////////////////////////////////////////////////////

kdTreeBuilderNaive::kdTreeBuilderNaive(void* observer,
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

////////////////////////////////////////////////////

static void deleteNode(kdNodeBuilder* node)
{
	if (node) {
		if (node->isLeaf)
			delete (kdLeafNodeBuilder*)node;
		else {
			deleteNode(((kdInnerNodeBuilder*)node)->left);
			deleteNode(((kdInnerNodeBuilder*)node)->right);
			delete (kdInnerNodeBuilder*)node;
		}
	}
}

kdTreeBuilderNaive::~kdTreeBuilderNaive()
{
	deleteNode(mRoot);
}

////////////////////////////////////////////////////

const BoundingBox& kdTreeBuilderNaive::boundingBox() const
{
	PR_ASSERT(mRoot, "Root has to be set before accessing it");
	return mRoot->boundingBox;
}

////////////////////////////////////////////////////

void kdTreeBuilderNaive::statElementsNode(kdNodeBuilder* node, size_t& sumV, float root_volume, uint32 depth)
{
	mDepth   = std::max(depth, mDepth);
	node->id = mNodeCount;
	++mNodeCount;

	if (node->isLeaf) {
		kdLeafNodeBuilder* leaf = (kdLeafNodeBuilder*)node;
		++mLeafCount;

		mMaxElementsPerLeaf = std::max(mMaxElementsPerLeaf, leaf->objects.size());
		mMinElementsPerLeaf = std::min(mMinElementsPerLeaf, leaf->objects.size());
		sumV += leaf->objects.size();

		const float ratio = leaf->boundingBox.surfaceArea() / root_volume;
		mExpectedLeavesVisited += ratio;
		mExpectedObjectsIntersected += leaf->objects.size() * ratio;
	} else {
		kdInnerNodeBuilder* inner = (kdInnerNodeBuilder*)node;
		if (inner->left) {
			statElementsNode(inner->left, sumV, root_volume, depth + 1);
		}

		if (inner->right) {
			statElementsNode(inner->right, sumV, root_volume, depth + 1);
		}

		mExpectedTraversalSteps += inner->boundingBox.surfaceArea() / root_volume;
	}
}

////////////////////////////////////////////////////

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

////////////////////////////////////////////////////

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
						size_t nl, size_t nr, size_t np, SidePlane& side,
						const BoundingBox& vl, const BoundingBox& vr)
{
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

static void classify(const std::vector<Primitive*>& objs,
					 uint32 dim, float v,
					 const BoundingBox& vl, const BoundingBox& vr,
					 uint32& nl, uint32& nr, uint32& np)
{
	nl = 0;
	nr = 0;
	np = 0;

	for (auto obj : objs) {
		BoundingBox dvl = obj->box;
		dvl.clipBy(vl);

		BoundingBox dvr = obj->box;
		dvr.clipBy(vr);

		const bool el = dvl.edge(dim) <= PR_EPSILON; // Empty left side
		const bool er = dvr.edge(dim) <= PR_EPSILON; // Empty right side
		if (el && er) {
			obj->side = S_Planar;
			++np;
		} else if (!el && er) {
			obj->side = S_Left;
			++nl;
		} else if (el && !er) {
			obj->side = S_Right;
			++nr;
		} else {
			obj->side = S_Both;
			++nl;
			++nr;
		}
	}
}

static void findSplit(float costIntersection, const std::vector<Primitive*>& objs,
					  const BoundingBox& V,
					  int& dim_out, float& v_out, float& c_out, SidePlane& side_out,
					  BoundingBox& vl_out, BoundingBox& vr_out)
{
	for (auto p : objs) {
		BoundingBox box = p->box;
		box.clipBy(V);
		for (int dim = 0; dim < 3; ++dim) {
			const bool planar = box.edge(dim) <= PR_EPSILON;

			for (int i = 0; i < (planar ? 1 : 2); ++i) {
				const float v = i == 0 ? box.lowerBound()(dim) : box.upperBound()(dim);

				BoundingBox vl, vr;
				splitBox(V, dim, v, vl, vr);

				// Skip non decreasing splits
				if (vl.edge(dim) <= PR_EPSILON || vr.edge(dim) <= PR_EPSILON)
					continue;

				uint32 nl, nr, np;
				classify(objs, dim, v, vl, vr, nl, nr, np);

				SidePlane side;
				const float c = SAH(costIntersection, V, nl, nr, np, side, vl, vr);

				if (c < c_out) {
					c_out	= c;
					dim_out  = dim;
					side_out = side;
					v_out	= v;
					vl_out   = vl;
					vr_out   = vr;
				}
			}
		}
	}
}

static void distributeObjects(const std::vector<Primitive*>& objs, SidePlane side,
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
		case S_Planar:
			if (side == SP_Left)
				left.push_back(obj);
			else
				right.push_back(obj);
			break;
		}
	}
}

inline static kdNodeBuilder* createLeafNode(void* observer,
										   kdTreeBuilderNaive::AddedCallback addedCallback,
										   std::vector<Primitive*>& objs,
										   const BoundingBox& V)
{
	kdLeafNodeBuilder* leaf = new kdLeafNodeBuilder(-1, V);
	for (auto obj : objs) {
		leaf->objects.push_back(obj->data);
		if (addedCallback)
			addedCallback(observer, obj->data, leaf->id);
	}
	return leaf;
}

static kdNodeBuilder* buildNode(void* observer,
								kdTreeBuilderNaive::CostCallback costCallback,
								kdTreeBuilderNaive::AddedCallback addedCallback,
								std::vector<Primitive*>& objs,
								const BoundingBox& V, uint32 depth, uint32 maxDepth, bool elementWise)
{
	if (objs.empty() || V.surfaceArea() <= PR_EPSILON || depth > maxDepth) {
		// Empty leaf or very tiny volume or max depth.
		// Just give up and make a leaf.
		return createLeafNode(observer, addedCallback, objs, V);
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

	int dim = 0;
	float v = 0;
	float c = std::numeric_limits<float>::infinity();
	SidePlane side;
	BoundingBox vl, vr;
	findSplit(costIntersection, objs, V, dim, v, c, side, vl, vr);

	if (c > objs.size() * costIntersection) {
		return createLeafNode(observer, addedCallback, objs, V);
	}

	uint32 nl, nr, np;
	classify(objs, dim, v, vl, vr, nl, nr, np);

	// Distribute
	std::vector<Primitive*> left, right;
	left.reserve(nl + (side == SP_Left ? np : 0));
	right.reserve(nr + (side == SP_Right ? np : 0));
	distributeObjects(objs, side, left, right);

	// Cleanup for memory
	std::vector<Primitive*>().swap(objs);

	return new kdInnerNodeBuilder(-1, dim, v,
								  buildNode(observer, costCallback, addedCallback, left, vl, depth + 1, maxDepth, elementWise),
								  buildNode(observer, costCallback, addedCallback, right, vr, depth + 1, maxDepth, elementWise),
								  V);
}

void kdTreeBuilderNaive::build(size_t size)
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

		mMaxDepth = 1;
		mDepth	= 1;
		mRoot	 = leaf;
		return;
	}

	mMaxDepth = std::ceil(8 + 1.5 * std::log2(size));

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

	mRoot = buildNode(mObserver, mCostCallback, mAddedCallback,
					  primitives, V, 0,
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

////////////////////////////////////////////////////

static void saveNode(std::ostream& stream, kdNodeBuilder* node)
{
	stream << node->id << " " << static_cast<uint32>(node->isLeaf ? 1 : 0);
	if (node->isLeaf) {
		kdLeafNodeBuilder* leafN = reinterpret_cast<kdLeafNodeBuilder*>(node);
		stream << " " << leafN->objects.size();
		for (const auto& entity : leafN->objects) {
			stream << " " << entity;
		}
		stream << std::endl;
	} else {
		kdInnerNodeBuilder* innerN = reinterpret_cast<kdInnerNodeBuilder*>(node);
		stream << " " << static_cast<uint32>(innerN->axis)
			   << " " << innerN->splitPos
			   << " " << innerN->left->id
			   << " " << innerN->right->id
			   << std::endl;

		saveNode(stream, innerN->left);
		saveNode(stream, innerN->right);
	}
}

void kdTreeBuilderNaive::save(std::ostream& stream) const
{
	stream << "pearray_kdtree" << std::endl;
	for (int i = 0; i < 3; ++i) {
		stream << mRoot->boundingBox.lowerBound()(i)
			   << " " << mRoot->boundingBox.upperBound()(i) << std::endl;
	}
	saveNode(stream, mRoot);
}

} // namespace PR
