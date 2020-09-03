// IWYU pragma: private, include "NTreeStackBuilder.h"

namespace PR {
template <size_t D, typename T, typename UniformT>
inline void NTreeStackBuilder<D, T, UniformT>::begin()
{
	if (mStack.empty()) {
		UniformIndex min, max, pivot;
		for (size_t i = 0; i < D; ++i) {
			min[i]	 = UniformT(0);
			max[i]	 = UniformT(1);
			pivot[i] = UniformT(0.5);
		}

		mTree		 = std::make_shared<Tree>();
		mTree->mRoot = std::make_unique<BranchNode>(UniformIndex(0.5));

		mStack.push_back(Context{ reinterpret_cast<BranchNode*>(mTree->mRoot.get()), 0, min, max });
	} else {
		BranchNode* node;
		UniformIndex min;
		UniformIndex max;
		{ // Inner group to make sure ctx is handled properly, even while the deconstructor should never be called
			auto& ctx = mStack.back();
			PR_ASSERT(ctx.Child < NodeCount::value, "begin() called exceeding child count");
			
			// Calculate pivot for this new child from min and max
			UniformIndex pivot;
			for (size_t i = 0; i < D; ++i) {
				min[i] = (ctx.Min[i] + ctx.Max[i]) / 2;
				max[i] = (ctx.Child & (1 << i)) ? ctx.Max[i] : ctx.Min[i];
				if (min[i] > max[i])
					std::swap(min[i], max[i]);
				pivot[i] = (min[i] + max[i]) / 2;
			}

			ctx.Node->mNodes[ctx.Child] = std::make_unique<BranchNode>(pivot);
			node						= reinterpret_cast<BranchNode*>(ctx.Node->mNodes[ctx.Child].get());
			ctx.Child += 1;
		}
		mStack.push_back(Context{ node, 0, min, max });
	}
}

template <size_t D, typename T, typename UniformT>
inline void NTreeStackBuilder<D, T, UniformT>::value(const T& v)
{
	if (mStack.empty()) {
		mTree		 = std::make_shared<Tree>();
		mTree->mRoot = std::make_unique<LeafNode>(UniformIndex(0.5), v);
	} else {
		auto& ctx = mStack.back();
		PR_ASSERT(ctx.Child < NodeCount::value, "value() called exceeding child count");
		ctx.Node->mNodes[ctx.Child] = std::make_unique<LeafNode>((ctx.Min + ctx.Max) / 2, v);
		ctx.Child += 1;
	}
}

template <size_t D, typename T, typename UniformT>
inline void NTreeStackBuilder<D, T, UniformT>::end()
{
	PR_ASSERT(mTree, "end() called before begin()");
	PR_ASSERT(!mStack.empty(), "end() called while stack is empty");
	PR_ASSERT(mStack.back().Child == NodeCount::value, "end() called while not all children where filled");
	mStack.pop_back();
}

template <size_t D, typename T, typename UniformT>
inline typename NTreeStackBuilder<D, T, UniformT>::TreePtr
NTreeStackBuilder<D, T, UniformT>::build() const
{
	PR_ASSERT(mTree, "build() called before construction finished");
	PR_ASSERT(mStack.empty(), "build() called before stack is empty");

	return mTree;
}
} // namespace PR
