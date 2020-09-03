#pragma once

#include "NTree.h"
#include <vector>

namespace PR {

/// Build a NTree with a stack driven approach
template <size_t D, typename T, typename UniformT = float>
class PR_LIB_BASE NTreeStackBuilder {
public:
	using Tree		= NTree<D, T, UniformT>;
	using TreePtr	= std::shared_ptr<Tree>;
	using ValueType = typename Tree::ValueType;
	using Dimension = std::integral_constant<size_t, D>;
	using NodeCount = std::integral_constant<size_t, (1 << D)>;

	NTreeStackBuilder()	= default;
	~NTreeStackBuilder() = default;

	inline void begin();
	inline void value(const T& v);
	inline void end();

	inline TreePtr build() const;

private:
	using LeafNode	   = typename Tree::LeafNode;
	using BranchNode   = typename Tree::BranchNode;
	using UniformIndex = typename Tree::UniformIndex;
	struct Context {
		BranchNode* Node;
		uint32 Child;
		UniformIndex Min;
		UniformIndex Max;
	};

	TreePtr mTree;
	std::vector<Context> mStack;
};

} // namespace PR

#include "NTreeStackBuilder.inl"