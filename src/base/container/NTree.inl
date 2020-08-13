// IWYU pragma: private, include "NTree.h"

namespace PR {
template <size_t D, typename T, typename UniformT>
inline T NTree<D, T, UniformT>::valueAt(const NTree<D, T, UniformT>::UniformIndex& idx) const
{
	if (!mRoot)
		return T(0);
	else if (mRoot->isLeaf())
		return reinterpret_cast<LeafNode*>(mRoot.get())->value();
	else
		return reinterpret_cast<BranchNode*>(mRoot.get())->valueAt(idx);
}

/*template <size_t D, typename T, typename UniformT>
inline void NTree<D, T, UniformT>::insertAt(const NTree<D, T, UniformT>::UniformIndex& idx, const T& value)
{
}*/

template <size_t D, typename T, typename UniformT>
inline typename NTree<D, T, UniformT>::LinearIndex
NTree<D, T, UniformT>::indexAt(const NTree<D, T, UniformT>::UniformIndex& idx) const
{
	if (!mRoot || mRoot->isLeaf())
		return 0;
	else
		return reinterpret_cast<BranchNode*>(mRoot.get())->indexAt(idx);
}

template <size_t D, typename T, typename UniformT>
inline T NTree<D, T, UniformT>::valueAt(const NTree<D, T, UniformT>::LinearIndex& idx) const
{
	if (!mRoot)
		return T(0);
	else if (mRoot->isLeaf())
		return reinterpret_cast<LeafNode*>(mRoot.get())->value();
	else
		return reinterpret_cast<BranchNode*>(mRoot.get())->valueAt(idx);
}

template <size_t D, typename T, typename UniformT>
inline void NTree<D, T, UniformT>::replaceAt(const NTree<D, T, UniformT>::LinearIndex& idx, const T& value)
{
	if (!mRoot)
		return;

	if (mRoot->isLeaf())
		reinterpret_cast<LeafNode*>(mRoot.get())->value() = value;
	else
		reinterpret_cast<BranchNode*>(mRoot.get())->replaceAt(idx, value);
}

////////////////////////////////////////////////////////////////
template <size_t D, typename T, typename UniformT>
inline void NTree<D, T, UniformT>::BranchNode::calculateTotalNodeCount()
{
	mTotalNodeCount = 0;
	for (size_t i = 0; i < mNodes.size(); ++i) {
		PR_ASSERT(mNodes[i], "Expected all nodes to be available");
		Node* node = mNodes[i].get();
		if (node->isLeaf())
			mTotalNodeCount += 1;
		else {
			reinterpret_cast<BranchNode*>(node)->calculateTotalNodeCount();
			mTotalNodeCount += reinterpret_cast<BranchNode*>(node)->totalNodeCount();
		}
	}
}
template <size_t D, typename T, typename UniformT>
inline T NTree<D, T, UniformT>::BranchNode::valueAt(const NTree<D, T, UniformT>::UniformIndex& idx) const
{
	const uint32 childindex = childIndexAt(idx);
	if (mNodes[childindex]->isLeaf())
		return reinterpret_cast<LeafNode*>(mNodes[childindex].get())->value();
	else
		return reinterpret_cast<BranchNode*>(mNodes[childindex].get())->valueAt(idx);
}

template <size_t D, typename T, typename UniformT>
inline typename NTree<D, T, UniformT>::LinearIndex
NTree<D, T, UniformT>::BranchNode::indexAt(const NTree<D, T, UniformT>::UniformIndex& idx) const
{
	const uint32 childindex = childIndexAt(idx);
	const LinearIndex pre	= linearIndexUntill(childindex);

	if (mNodes[childindex]->isLeaf())
		return pre + 1;
	else
		return pre + reinterpret_cast<BranchNode*>(mNodes[childindex].get())->indexAt(idx);
}

template <size_t D, typename T, typename UniformT>
inline T NTree<D, T, UniformT>::BranchNode::valueAt(const NTree<D, T, UniformT>::LinearIndex& idx) const
{
	size_t preNodeCount;
	const uint32 childindex = childIndexAt(idx, preNodeCount);
	if (mNodes[childindex]->isLeaf())
		return reinterpret_cast<LeafNode*>(mNodes[childindex].get())->value();
	else
		return reinterpret_cast<BranchNode*>(mNodes[childindex].get())->valueAt(idx - preNodeCount);
}

template <size_t D, typename T, typename UniformT>
inline void NTree<D, T, UniformT>::BranchNode::replaceAt(const NTree<D, T, UniformT>::LinearIndex& idx, const T& value)
{
	size_t preNodeCount;
	const uint32 childindex = childIndexAt(idx, preNodeCount);
	if (mNodes[childindex]->isLeaf())
		reinterpret_cast<LeafNode*>(mNodes[childindex].get())->value() = value;
	else
		reinterpret_cast<BranchNode*>(mNodes[childindex].get())->replaceAt(idx - preNodeCount, value);
}

template <size_t D, typename T, typename UniformT>
inline size_t NTree<D, T, UniformT>::BranchNode::childIndexAt(const NTree<D, T, UniformT>::UniformIndex& idx) const
{
	uint32 childindex = 0;
	for (size_t i = 0; i < D; ++i)
		childindex |= (idx[i] >= mPivot[i]) * (1 << i);

	PR_ASSERT(childindex < mNodes.size(), "Expected valid child index");
	return childindex;
}

template <size_t D, typename T, typename UniformT>
inline size_t NTree<D, T, UniformT>::BranchNode::childIndexAt(const NTree<D, T, UniformT>::LinearIndex& idx, size_t& preNodeCount) const
{
	preNodeCount = 0;
	for (size_t i = 0; i < mNodes.size(); ++i) {
		PR_ASSERT(mNodes[i], "Expected all nodes to be available");
		Node* node = mNodes[i].get();
		uint32_t ind;
		if (node->isLeaf())
			ind = 1;
		else
			ind = reinterpret_cast<BranchNode*>(node)->totalNodeCount();

		if (preNodeCount + ind <= idx)
			preNodeCount += ind;
		else
			return i;
	}

	PR_ASSERT(false, "Invalid configuration found");
	return 0;
}

template <size_t D, typename T, typename UniformT>
inline typename NTree<D, T, UniformT>::LinearIndex
NTree<D, T, UniformT>::BranchNode::linearIndexUntill(size_t childIndex) const
{
	LinearIndex ind = 0;
	for (size_t i = 0; i < childIndex; ++i) {
		PR_ASSERT(mNodes[i], "Expected all nodes to be available");
		Node* node = mNodes[i].get();
		if (node->isLeaf())
			ind += 1;
		else
			ind += reinterpret_cast<BranchNode*>(node)->totalNodeCount();
	}

	PR_ASSERT(ind < mTotalNodeCount, "Expected partial node count to be less than total node count");
	return ind;
}

template <size_t D, typename T, typename UniformT>
inline void NTree<D, T, UniformT>::BranchNode::calculateStats(size_t depth, size_t& maxDepth, size_t& leafCount, size_t& branchCount)
{
	maxDepth = std::max(maxDepth, depth);
	for (size_t i = 0; i < mNodes.size(); ++i) {
		PR_ASSERT(mNodes[i], "Expected all nodes to be available");
		Node* node = mNodes[i].get();
		if (node->isLeaf())
			++leafCount;
		else {
			++branchCount;
			reinterpret_cast<BranchNode*>(node)->calculateStats(depth + 1, maxDepth, leafCount, branchCount);
		}
	}
}

////////////////////////////////////////////////////////////////

template <size_t D, typename T, typename UniformT>
inline void NTree<D, T, UniformT>::bake()
{
	if (!mRoot->isLeaf()) {
		mMaxDepth		 = 0;
		mLeafNodeCount	 = 0;
		mBranchNodeCount = 1;

		reinterpret_cast<BranchNode*>(mRoot.get())->calculateTotalNodeCount();
		reinterpret_cast<BranchNode*>(mRoot.get())->calculateStats(1, mMaxDepth, mLeafNodeCount, mBranchNodeCount);
	} else {
		mMaxDepth		 = 0;
		mLeafNodeCount	 = 1;
		mBranchNodeCount = 0;
	}
}
} // namespace PR