#pragma once

#include "geometry/BoundingBox.h"

/*
 Reference kdTree implementation with bad performance O(N^2)
*/
namespace PR {
class Serializer;
class PR_LIB kdTreeBuilderNaive {
public:
	typedef BoundingBox (*GetBoundingBoxCallback)(void*, size_t);
	typedef float (*CostCallback)(void*, size_t);
	typedef void (*AddedCallback)(void*, size_t, size_t id);

	kdTreeBuilderNaive(void* observer,
					   GetBoundingBoxCallback getBoundingBox,
					   CostCallback cost,
					   AddedCallback addedCallback = nullptr);

	virtual ~kdTreeBuilderNaive();

	inline bool costElementWise() const { return mElementWise; }
	inline void setCostElementWise(bool b) { mElementWise = b; }

	inline uint32 depth() const { return mDepth; }
	inline float avgElementsPerLeaf() const { return mAvgElementsPerLeaf; }
	inline size_t minElementsPerLeaf() const { return mMinElementsPerLeaf; }
	inline size_t maxElementsPerLeaf() const { return mMaxElementsPerLeaf; }
	inline size_t leafCount() const { return mLeafCount; }
	inline float expectedTraversalSteps() const { return mExpectedTraversalSteps; }
	inline float expectedLeavesVisited() const { return mExpectedLeavesVisited; }
	inline float expectedObjectsIntersected() const { return mExpectedObjectsIntersected; }

	inline bool isEmpty() const { return mRoot == nullptr || mDepth == 0; }

	const BoundingBox& boundingBox() const;

	void build(size_t size, bool withStats = true);
	void save(Serializer& stream) const;

private:
	void statElementsNode(struct kdNodeBuilderNaive* node, size_t& sumV, float root_volume, uint32 depth);

	struct kdNodeBuilderNaive* mRoot;

	void* mObserver;

	size_t mNodeCount;
	GetBoundingBoxCallback mGetBoundingBox;
	CostCallback mCostCallback;
	AddedCallback mAddedCallback;

	bool mElementWise;
	uint32 mMaxDepth;
	uint32 mDepth;
	float mAvgElementsPerLeaf;
	size_t mMinElementsPerLeaf;
	size_t mMaxElementsPerLeaf;
	size_t mLeafCount;

	float mExpectedTraversalSteps;
	float mExpectedLeavesVisited;
	float mExpectedObjectsIntersected;
};
} // namespace PR
