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

template <size_t D, typename T, typename UniformT>
inline void NTree<D, T, UniformT>::insertAt(const NTree<D, T, UniformT>::UniformIndex& idx, const T& value)
{
	if (!mRoot)
		mRoot = std::make_unique<LeafNode>(idx, value);
	else if (mRoot->isLeaf()) { // Switch leaf to branch
		auto oldLeaf = reinterpret_cast<LeafNode*>(mRoot.get());
		auto branch	 = std::make_unique<BranchNode>(UniformIndex(0.5));
		branch->insertAt(idx, value, UniformIndex(0), UniformIndex(1));
		branch->insertAt(oldLeaf->index(), oldLeaf->value(), UniformIndex(0), UniformIndex(1));
		mRoot = std::move(branch);
	} else
		reinterpret_cast<BranchNode*>(mRoot.get())->insertAt(idx, value, UniformIndex(0), UniformIndex(1));
}

template <size_t D, typename T, typename UniformT>
inline T NTree<D, T, UniformT>::nearestValueAt(const NTree<D, T, UniformT>::UniformIndex& idx, UniformT maxRadius) const
{
	if (!mRoot)
		return T(0);
	else if (mRoot->isLeaf()) {
		if (reinterpret_cast<LeafNode*>(mRoot.get())->index().matrix().squaredNorm() <= maxRadius * maxRadius)
			return reinterpret_cast<LeafNode*>(mRoot.get())->value();
		else
			return T(0);
	} else {
		UniformT currentRadius = maxRadius;
		return reinterpret_cast<BranchNode*>(mRoot.get())->nearestValueAt(idx, currentRadius);
	}
}

template <size_t D, typename T, typename UniformT>
inline size_t NTree<D, T, UniformT>::totalNodeCount() const { return leafNodeCount() + branchNodeCount(); }

template <size_t D, typename T, typename UniformT>
inline size_t NTree<D, T, UniformT>::leafNodeCount() const
{
	if (!mRoot)
		return 0;
	else if (mRoot->isLeaf())
		return 1;
	else
		return reinterpret_cast<BranchNode*>(mRoot.get())->leafNodeCount();
}

template <size_t D, typename T, typename UniformT>
inline size_t NTree<D, T, UniformT>::branchNodeCount() const
{
	if (!mRoot)
		return 0;
	else if (mRoot->isLeaf())
		return 0;
	else
		return reinterpret_cast<BranchNode*>(mRoot.get())->branchNodeCount() + 1;
}

template <size_t D, typename T, typename UniformT>
inline size_t NTree<D, T, UniformT>::maxDepth() const
{
	if (!mRoot)
		return 0;
	else if (mRoot->isLeaf())
		return 0;
	else
		return reinterpret_cast<BranchNode*>(mRoot.get())->maxDepth() + 1;
}

template <size_t D, typename T, typename UniformT>
inline void NTree<D, T, UniformT>::clear()
{
	mRoot.reset();
}

template <size_t D, typename T, typename UniformT>
template <typename Func>
inline void NTree<D, T, UniformT>::visit(const Func& func)
{
	if (!mRoot)
		return;
	else if (mRoot->isLeaf())
		func(reinterpret_cast<LeafNode*>(mRoot.get())->value());
	else
		reinterpret_cast<BranchNode*>(mRoot.get())->visit(func);
}

template <size_t D, typename T, typename UniformT>
template <typename Func>
inline void NTree<D, T, UniformT>::visit(const Func& func) const
{
	if (!mRoot)
		return;
	else if (mRoot->isLeaf())
		func(reinterpret_cast<LeafNode*>(mRoot.get())->value());
	else
		reinterpret_cast<BranchNode*>(mRoot.get())->visit(func);
}

template <size_t D, typename T, typename UniformT>
template <typename Func>
inline void NTree<D, T, UniformT>::enumerate(const Func& func) const
{
	if (!mRoot)
		return;
	else if (mRoot->isLeaf())
		func(reinterpret_cast<LeafNode*>(mRoot.get())->index(),
			 reinterpret_cast<LeafNode*>(mRoot.get())->value());
	else
		reinterpret_cast<BranchNode*>(mRoot.get())->enumerate(func);
}
////////////////////////////////////////////////////////////////
template <size_t D, typename T, typename UniformT>
inline T NTree<D, T, UniformT>::BranchNode::valueAt(const NTree<D, T, UniformT>::UniformIndex& idx) const
{
	const uint32 childindex = childIndexAt(idx);
	if (!mNodes[childindex])
		return T(0);
	else if (mNodes[childindex]->isLeaf())
		return reinterpret_cast<LeafNode*>(mNodes[childindex].get())->value();
	else
		return reinterpret_cast<BranchNode*>(mNodes[childindex].get())->valueAt(idx);
}

template <size_t D, typename T, typename UniformT>
inline void NTree<D, T, UniformT>::BranchNode::insertAt(const NTree<D, T, UniformT>::UniformIndex& idx, const T& value,
														const NTree<D, T, UniformT>::UniformIndex& low, const NTree<D, T, UniformT>::UniformIndex& high)
{
	const uint32 childindex = childIndexAt(idx);
	if (!mNodes[childindex]) {
		mNodes[childindex] = std::make_unique<LeafNode>(idx, value);
	} else if (mNodes[childindex]->isLeaf()) { // Switch leaf to branch
		UniformIndex nlow  = low;
		UniformIndex nhigh = high;
		adaptFor(idx, nlow, nhigh);

		auto oldLeaf = reinterpret_cast<LeafNode*>(mNodes[childindex].get());
		auto branch	 = std::make_unique<BranchNode>((nhigh + nlow) / 2);
		branch->insertAt(idx, value, nlow, nhigh);
		branch->insertAt(oldLeaf->index(), oldLeaf->value(), nlow, nhigh);
		mNodes[childindex] = std::move(branch);
	} else {
		UniformIndex nlow  = low;
		UniformIndex nhigh = high;
		adaptFor(idx, nlow, nhigh);
		reinterpret_cast<BranchNode*>(mNodes[childindex].get())->insertAt(idx, value, nlow, nhigh);
	}
}

template <size_t D, typename T, typename UniformT>
inline T NTree<D, T, UniformT>::BranchNode::nearestValueAt(const NTree<D, T, UniformT>::UniformIndex& idx, UniformT& maxRadius) const
{
	T currentValue = T(0);

	// First check respective index if available
	const uint32 childindex = childIndexAt(idx);
	if (mNodes[childindex] && mNodes[childindex]->isLeaf()) {
		UniformIndex point = reinterpret_cast<LeafNode*>(mNodes[childindex].get())->index();
		UniformT dist2	   = (point - idx).matrix().squaredNorm();
		if (dist2 < maxRadius * maxRadius) {
			maxRadius	 = std::sqrt(dist2);
			currentValue = reinterpret_cast<LeafNode*>(mNodes[childindex].get())->value();
		}
	} else if (mNodes[childindex] && !mNodes[childindex]->isLeaf()) {
		UniformT newR = maxRadius;
		T newT		  = reinterpret_cast<BranchNode*>(mNodes[childindex].get())->nearestValueAt(idx, newR);
		if (newR < maxRadius) {
			maxRadius	 = newR;
			currentValue = newT;
		}
	}

	// Calculate intersected edges
	const UniformIndex idxM = (idx >= mPivot).select(idx - maxRadius, idx + maxRadius);
	size_t intersectionMap	= 0;
	for (size_t j = 0; j < D; ++j) {
		const bool hit = (mPivot[j] >= idx[j] && mPivot[j] < idxM[j]) || (mPivot[j] <= idx[j] && mPivot[j] > idxM[j]);
		intersectionMap |= hit * (1 << j);
	}

	// Check other parts if necessary
	for (size_t i = 0; i < mNodes.size(); ++i) {
		// If this quadrant is hit
		// (first map such that indices are coherent to each other then check if necessary dimension is intersected)
		const bool hit = intersectionMap & (i ^ childindex);
		if (!mNodes[i] || !hit)
			continue;
		else if (mNodes[i]->isLeaf()) {
			UniformIndex point = reinterpret_cast<LeafNode*>(mNodes[i].get())->index();
			UniformT dist2	   = (point - idx).matrix().squaredNorm();
			if (dist2 < maxRadius * maxRadius) {
				maxRadius	 = std::sqrt(dist2);
				currentValue = reinterpret_cast<LeafNode*>(mNodes[i].get())->value();
			}
		} else {
			UniformT newR = maxRadius;
			T newT		  = reinterpret_cast<BranchNode*>(mNodes[i].get())->nearestValueAt(idx, newR);
			if (newR < maxRadius) {
				maxRadius	 = newR;
				currentValue = newT;
			}
		}
	}
	return currentValue;
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
void NTree<D, T, UniformT>::BranchNode::adaptFor(const NTree<D, T, UniformT>::UniformIndex& idx,
												 NTree<D, T, UniformT>::UniformIndex& low, NTree<D, T, UniformT>::UniformIndex& high) const
{
	for (size_t i = 0; i < D; ++i) {
		if (idx[i] >= mPivot[i])
			low[i] = mPivot[i];
		else
			high[i] = mPivot[i];
	}
}

template <size_t D, typename T, typename UniformT>
inline size_t NTree<D, T, UniformT>::BranchNode::leafNodeCount() const
{
	size_t leafCount = 0;
	for (size_t i = 0; i < mNodes.size(); ++i) {
		if (!mNodes[i])
			continue;
		else if (mNodes[i]->isLeaf())
			++leafCount;
		else
			leafCount += reinterpret_cast<BranchNode*>(mNodes[i].get())->leafNodeCount();
	}
	return leafCount;
}

template <size_t D, typename T, typename UniformT>
inline size_t NTree<D, T, UniformT>::BranchNode::branchNodeCount() const
{
	size_t branchCount = 0;
	for (size_t i = 0; i < mNodes.size(); ++i) {
		if (mNodes[i] && !mNodes[i]->isLeaf())
			branchCount += reinterpret_cast<BranchNode*>(mNodes[i].get())->branchNodeCount() + 1;
	}
	return branchCount;
}

template <size_t D, typename T, typename UniformT>
inline size_t NTree<D, T, UniformT>::BranchNode::maxDepth() const
{
	size_t maxDepth = 0;
	for (size_t i = 0; i < mNodes.size(); ++i) {
		if (mNodes[i] && !mNodes[i]->isLeaf())
			maxDepth = std::max(maxDepth, reinterpret_cast<BranchNode*>(mNodes[i].get())->maxDepth() + 1);
	}
	return maxDepth;
}

template <size_t D, typename T, typename UniformT>
template <typename Func>
inline void NTree<D, T, UniformT>::BranchNode::visit(const Func& func)
{
	for (size_t i = 0; i < mNodes.size(); ++i) {
		if (!mNodes[i])
			continue;
		else if (mNodes[i]->isLeaf())
			func(reinterpret_cast<LeafNode*>(mNodes[i].get())->value());
		else
			reinterpret_cast<BranchNode*>(mNodes[i].get())->visit(func);
	}
}

template <size_t D, typename T, typename UniformT>
template <typename Func>
inline void NTree<D, T, UniformT>::BranchNode::visit(const Func& func) const
{
	for (size_t i = 0; i < mNodes.size(); ++i) {
		if (!mNodes[i])
			continue;
		else if (mNodes[i]->isLeaf())
			func(reinterpret_cast<LeafNode*>(mNodes[i].get())->value());
		else
			reinterpret_cast<BranchNode*>(mNodes[i].get())->visit(func);
	}
}

template <size_t D, typename T, typename UniformT>
template <typename Func>
inline void NTree<D, T, UniformT>::BranchNode::enumerate(const Func& func) const
{
	for (size_t i = 0; i < mNodes.size(); ++i) {
		if (!mNodes[i])
			continue;
		else if (mNodes[i]->isLeaf())
			func(reinterpret_cast<LeafNode*>(mNodes[i].get())->index(),
				 reinterpret_cast<LeafNode*>(mNodes[i].get())->value());
		else
			reinterpret_cast<BranchNode*>(mNodes[i].get())->enumerate(func);
	}
}
} // namespace PR